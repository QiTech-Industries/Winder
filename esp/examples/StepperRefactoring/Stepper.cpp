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
    _driver->sgt(_stallValue);
    _driver->sfilt(true);

    // StallGuard/Coolstep config
    _driver->TCOOLTHRS(1000000);
    _driver->semin(0);
    _driver->semax(1);

    // FastAccelStepper config
    _stepper = _engine.stepperConnectToPin(config.pins.step);
    _stepper->setDirectionPin(config.pins.dir);
    _stepper->setEnablePin(config.pins.en);
    _stepper->setAcceleration(_acceleration);
}

bool Stepper::isMoving() { return _stepper->isRampGeneratorActive(); }

void Stepper::printStatus() {
    char mode[20];
    modeToString(_currentRecipe.mode, mode);
    Serial.printf("%s - Mode: %s Speed: %.2frpm  Load: %u%%\n",
                  _config.stepperId, mode, _current.rpm, _current.load);
}

uint16_t Stepper::getCurrentStall() {
    DRV_STATUS_t drvStatus{0};
    drvStatus.sr = _driver->DRV_STATUS();
    return drvStatus.sg_result;
}

void Stepper::updateStatus() {
    _current.rpm =
        speedUsToRpm(_stepper->getCurrentSpeedInUs(), _microstepsPerRotation);
    _current.load =
        stallToLoadPercent(abs(_stepper->getCurrentSpeedInUs()), getCurrentStall(),
                           speeds, minLoad, maxLoad, 40);
    _current.position =
        positionToMm(_stepper->getCurrentPosition(), _microstepsPerRotation,
                     _config.mmPerRotation);
}

void Stepper::forceStop() {
    _stepper->forceStopAndNewPosition(_stepper->getCurrentPosition());
}

bool Stepper::needsHome(mode_e targetMode, mode_e currentMode) {
    if ((targetMode == POSITIONING || targetMode == OSCILLATINGLEFT ||
         targetMode == OSCILLATINGRIGHT) &&
        currentMode != HOMING) {
        return true;
    }
    return false;
}

bool Stepper::isRecipeFinished() {
    if (_currentRecipe.mode == POSITIONING ||
        _currentRecipe.mode == OSCILLATINGLEFT ||
        _currentRecipe.mode == OSCILLATINGRIGHT) {
        return !_stepper->isRampGeneratorActive();
    }
    if (_currentRecipe.mode == HOMING && _startSpeedReached) {
        return _current.load == 100;
    }
    return false;
}

bool Stepper::switchCurrentRecipeMode() {
    if (_currentRecipe.mode == _targetRecipe.mode && !isRecipeFinished())
        return false;
    if (_currentRecipe.mode == HOMING && !isRecipeFinished()) return false;

    if (needsHome(_targetRecipe.mode, _currentRecipe.mode)) {
        _currentRecipe = _defaultRecipe;
        _currentRecipe.mode = HOMING;
        _currentRecipe.rpm = 40;
        _startSpeedReached = false;
        return true;
    }
    if (isRecipeFinished() && _currentRecipe.mode == OSCILLATINGLEFT) {
        _currentRecipe.mode = OSCILLATINGRIGHT;
        return true;
    }
    if (isRecipeFinished() && _currentRecipe.mode == _targetRecipe.mode) {
        _targetRecipe = _defaultRecipe;
        _targetRecipe.mode = STANDBY;
        return true;
    }
    _currentRecipe = _targetRecipe;
    return true;
}

void Stepper::startRecipe() {
    if (_targetRecipe.mode == OFF) {
        _driver->toff(0);
        return;
    }

    _driver->toff(1);
    forceStop();
    _stepper->setSpeedInUs(
        speedRpmToUs(_currentRecipe.rpm, _microstepsPerRotation));
    _stepper->applySpeedAcceleration();

    if (_currentRecipe.mode == POSITIONING ||
        _currentRecipe.mode == OSCILLATINGLEFT) {
        _stepper->moveTo(mmToPosition(_currentRecipe.position1,
                                      _microstepsPerRotation,
                                      _config.mmPerRotation));
        return;
    }
    if (_currentRecipe.mode == OSCILLATINGRIGHT) {
        _stepper->moveTo(mmToPosition(_currentRecipe.position2,
                                      _microstepsPerRotation,
                                      _config.mmPerRotation));
        return;
    }

    if (_currentRecipe.rpm < 0) {
        _stepper->runForward();
        return;
    }
    if (_currentRecipe.rpm > 0) {
        _stepper->runBackward();
        return;
    }
}

void Stepper::tuneCurrentRecipe() {
    if (abs(_current.rpm) >= abs(_currentRecipe.rpm)) _startSpeedReached = true;
    if (_currentRecipe.mode != ADJUSTING) return;
    if (!_startSpeedReached) return;

    if (_current.load < _currentRecipe.load) {
        // load too low speed up
        _stepper->setSpeedInUs(_stepper->getSpeedInUs() * 0.9);
        _stepper->applySpeedAcceleration();
        return;
    }
    if (_current.load > _currentRecipe.load) {
        // load too high slow down
        _stepper->setSpeedInUs(_stepper->getSpeedInUs() * 1.5);
        _stepper->applySpeedAcceleration();
        return;
    }
}

// Synchronous methods
void Stepper::oscillate(float rpm, int32_t leftPos, int32_t rightPos) {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = OSCILLATINGLEFT;
    _targetRecipe.rpm = rpm;
    _targetRecipe.position1 = leftPos;
    _targetRecipe.position2 = rightPos;
}

void Stepper::position(float rpm, int32_t position) {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = POSITIONING;
    _targetRecipe.rpm = rpm;
    _targetRecipe.position1 = position;
}

void Stepper::rotate(float rpm) {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = ROTATING;
    _targetRecipe.rpm = rpm;
}

void Stepper::adjustSpeedToLoad(float startSpeed, uint8_t desiredLoad) {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = ADJUSTING;
    _targetRecipe.rpm = startSpeed;
    _targetRecipe.load = desiredLoad;
    _startSpeedReached = false;
}

void Stepper::home(float rpm) {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = HOMING;
    _targetRecipe.rpm = rpm;
    _startSpeedReached = false;
}

void Stepper::standby() {
    _targetRecipe = _defaultRecipe;
    _targetRecipe.mode = STANDBY;
}

void Stepper::off() { _targetRecipe = _defaultRecipe; }

void Stepper::handle() {
    if (switchCurrentRecipeMode())
        startRecipe();
    else
        tuneCurrentRecipe();

    updateStatus();
    printStatus();
}