#include <Stepper.h>

using TMC2130_n::DRV_STATUS_t;

Stepper::Stepper() {}

void Stepper::init(stepper_s config) {
    _config = config;
    _microstepsPerRotation = config.stepsPerRotation * config.microstepsPerStep;
    _driver = new TMC2130Stepper(config.pins.cs);
    _driver->begin();
    _engine.init();

    // DRIVER config
    _driver->toff(0);
    _driver->blank_time(5);
    _driver->rms_current(config.maxCurrent);
    _driver->microsteps(config.microstepsPerStep);
    _driver->sgt(DRIVER_STALL_VALUE);
    _driver->sfilt(true);

    // StallGuard/Coolstep config
    _driver->TCOOLTHRS(1000000);
    _driver->semin(0);
    _driver->semax(1);

    // FastAccelStepper config
    _stepper = _engine.stepperConnectToPin(config.pins.step);
    _stepper->setDirectionPin(config.pins.dir);
    _stepper->setEnablePin(config.pins.en);
    _stepper->setAcceleration(DEFAULT_ACCELERATION);
}

bool Stepper::isMoving() { return _stepper->isRampGeneratorActive(); }

void Stepper::printStatus(bool verbous = false) {
    // Resolve name of recipe-modes
    char modeCurrent[20];
    modeToString(_currentRecipe.mode, modeCurrent);
    char modeTarget[20];
    modeToString(_targetRecipe.mode, modeTarget);

    // Print info
    Serial.printf("{time: %lu, summary: {id: '%s', mode: '%s', rpm: %.2f, load: %u%%, pos: %d, homed: %d}",
        millis(), _config.stepperId, modeCurrent, _stepperStatus.rpm, _stepperStatus.load, _stepperStatus.position, _homed);
    if(verbous){
        Serial.printf(", errors: 'TODO'"); // TODO: Implement
        Serial.printf(", recipeNow: {mode: '%s', rpm: %.2f, load: %d, pos1: %zu, pos2: %zu}",
            modeCurrent, _currentRecipe.rpm, _currentRecipe.load, _currentRecipe.position1, _currentRecipe.position2);
        Serial.printf(", recipeTarget: {mode: '%s', rpm: %.2f, load: %d, pos1: %zu, pos2: %zu}",
            modeTarget, _targetRecipe.rpm, _targetRecipe.load, _targetRecipe.position1, _targetRecipe.position2);
    }
    Serial.println("}");

}

uint16_t Stepper::getCurrentStall() {
    DRV_STATUS_t drvStatus{0};
    drvStatus.sr = _driver->DRV_STATUS();
    return drvStatus.sg_result;
}

void Stepper::setDebugging(bool debugging){
    _debugging = debugging;
}

void Stepper::updateStatus() {
    _stepperStatus.rpm =
        speedUsToRpm(_stepper->getCurrentSpeedInUs(), _microstepsPerRotation);
    _stepperStatus.load =
        stallToLoadPercent(abs(_stepper->getCurrentSpeedInUs()), getCurrentStall(),
                           speeds, minLoad, maxLoad, 40);
    _stepperStatus.position =
        positionToMm(_stepper->getCurrentPosition(), _microstepsPerRotation,
                     _config.mmPerRotation);
}

void Stepper::forceStop() {
    _stepper->forceStopAndNewPosition(_stepper->getCurrentPosition());
}

bool Stepper::checkNeedsHome(mode_e targetMode, mode_e currentMode) {
    if(_homed) return false;
    if(currentMode == HOMING) return false;

    switch(targetMode){
        case POSITIONING:
        case OSCILLATINGLEFT:
        case OSCILLATINGRIGHT:
            return true;
        default:
            return false;
    };
}

bool Stepper::isRecipeFinished() {
    if (_currentRecipe.mode == POSITIONING ||
        _currentRecipe.mode == OSCILLATINGLEFT ||
        _currentRecipe.mode == OSCILLATINGRIGHT) {
        return !_stepper->isRampGeneratorActive();
    }
    if (_currentRecipe.mode == HOMING && isStartSpeedReached()) {
        return _stepperStatus.load == 100;
    }
    return false;
}

void Stepper::applySpeed(float speedRpm){
    _driver->toff(1);
    forceStop();
    if(speedRpm != 0) {
        _stepper->setSpeedInUs(
        speedRpmToUs(speedRpm, _microstepsPerRotation));
        _stepper->applySpeedAcceleration();

        if(speedRpm < 0){
            _stepper->runForward();
        } else {
            _stepper->runBackward();
        }
    }
}

void Stepper::startRecipe(recipe_s recipe) {
    if (recipe.mode == OFF) {
        _driver->toff(0);
        return;
    }

    // Set new speed
    applySpeed(recipe.rpm);

    // Do mode specific stuff
    switch(recipe.mode){
        case ROTATING:
        case ADJUSTING:
        case HOMING:
            applySpeed(recipe.rpm);
            break;
        case POSITIONING:
        case OSCILLATINGLEFT:
            applySpeed(recipe.rpm);
            _stepper->moveTo(mmToPosition(recipe.position1,
                _microstepsPerRotation,
                _config.mmPerRotation)
            );
            break;
        case OSCILLATINGRIGHT:
            applySpeed(recipe.rpm);
            _stepper->moveTo(mmToPosition(recipe.position2,
                _microstepsPerRotation,
                _config.mmPerRotation)
            );
            break;
        case OFF:
            applySpeed(0); // Stop any movement
            _driver->toff(0); // power off the driver
            break;
        case STANDBY:
            applySpeed(0); // Stop any movement
            break;
    }
}

bool Stepper::isStartSpeedReached(){
    return abs(_stepperStatus.rpm) >= abs(_currentRecipe.rpm);
}

void Stepper::adjustSpeedByLoad() {
    if (!isStartSpeedReached()) return; // Invalid load-measurements for low speeds

    if (_stepperStatus.load < _currentRecipe.load) {
        // load too low speed up
        _stepper->setSpeedInUs(_stepper->getSpeedInUs() * 0.9);
        _stepper->applySpeedAcceleration();
        return;
    }
    if (_stepperStatus.load > _currentRecipe.load) {
        // load too high slow down
        _stepper->setSpeedInUs(_stepper->getSpeedInUs() * 1.5);
        _stepper->applySpeedAcceleration();
        return;
    }
}

// Synchronous methods
void Stepper::moveOscillate(float rpm, int32_t leftPos, int32_t rightPos, bool directionLeft) {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = directionLeft ? OSCILLATINGLEFT : OSCILLATINGRIGHT;
    _targetRecipe.rpm = rpm;
    _targetRecipe.position1 = leftPos;
    _targetRecipe.position2 = rightPos;
    _newCommand = true;
}

void Stepper::movePosition(float rpm, int32_t position) {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = POSITIONING;
    _targetRecipe.rpm = rpm;
    _targetRecipe.position1 = position;
    _newCommand = true;
}

void Stepper::moveRotate(float rpm) {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = ROTATING;
    _targetRecipe.rpm = rpm;
    _newCommand = true;
}

void Stepper::moveRotateWithLoadAdjust(float startSpeed, uint8_t desiredLoad) {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = ADJUSTING;
    _targetRecipe.rpm = startSpeed;
    _targetRecipe.load = desiredLoad;
    _newCommand = true;
}

void Stepper::moveHome(float rpm) {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = HOMING;
    _targetRecipe.rpm = rpm;
    _newCommand = true;
}

void Stepper::switchModeStandby() {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = STANDBY;
    _newCommand = true;
}

void Stepper::switchModeOff() {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = OFF;
    _newCommand = true;
}

void Stepper::handle() {
    // Switch recipe on new command, unless we are still homing. OFF has priority for safety reasons though
    if(_newCommand && (_targetRecipe.mode == OFF || _currentRecipe.mode != HOMING)){
        // Determine next command
        if(checkNeedsHome(_targetRecipe.mode, _currentRecipe.mode)){
            // Initiate homing instead of next command
            _currentRecipe = _defaultRecipe;
            _currentRecipe.mode = HOMING;
            _currentRecipe.rpm = HOMING_SPEED_RPM;
            _newCommand = true;
        } else {
            _currentRecipe = _targetRecipe;
            _targetRecipe = _defaultRecipe;
            _newCommand = false;
        }
        
        startRecipe(_currentRecipe);
    }

    // Handle the current recipe, that was already started at some point in the past
    switch(_currentRecipe.mode){
        case HOMING:
            // Wait for stopper to be hit to set home
            if (isStartSpeedReached() && _stepperStatus.load == 100) {
                _stepper->forceStopAndNewPosition(0);
                _homed = true;
                _currentRecipe.mode = STANDBY;
                _newCommand = true; // Return to whatever we were doing on the next cycle
            }
            break;
        case ADJUSTING:
            adjustSpeedByLoad();
            break;
        case POSITIONING:
            // Wait for motor to stop moving, as it means we reached our destination
            if(!_stepper->isRampGeneratorActive()){
                switchModeStandby();
            }
            break;
        case OSCILLATINGLEFT:
        case OSCILLATINGRIGHT:
            // Invert direction when we have reached our destination
            if(!_stepper->isRampGeneratorActive()){
                moveOscillate(_currentRecipe.rpm, _currentRecipe.position1, _currentRecipe.position2, (_currentRecipe.mode != OSCILLATINGLEFT));
            }
            break;
        case ROTATING: // Keep on rolling, nothing to do here
        case STANDBY:  // Nothing to do here, too
        case OFF:  // Literally nothing to do here
        default: // Should never happen
            break;
    };

    // Stats and logging
    updateStatus();
    if(_debugging) printStatus();
}