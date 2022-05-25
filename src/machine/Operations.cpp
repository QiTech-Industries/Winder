#include <machine/Operations.h>

void operatePull(float speedRpm) {
    logPrint(INFO, INFO, "Winder::operatePull speedRpm: %.2f, oscillation: {start: %d, end: %d, offset: %d}\n", speedRpm,
             _softC.ferrari_min, _softC.ferrari_max, _hardC.motors.ferrariOffset);

    // Rotate puller, disable spool, position ferrari at start position for following winding
    _stepperSpool->switchModeOff();
    _stepperFerrari->movePosition(-FERRARI_POSITIONING_SPEED, _softC.ferrari_max - _hardC.motors.ferrariOffset);
    if (_currentWinderOperation == OPERATE_PULLING || _currentWinderOperation == OPERATE_WINDING) {
        // Keep rotating without interrupt
        _stepperPuller->adjustMoveSpeed(-speedRpm);
    } else {
        _stepperPuller->moveRotate(-speedRpm);
    }
    _currentWinderOperation = OPERATE_PULLING;
}

void operateUnwind(float speedMetersPerMinute) {
    const float FERRARI_POSITIONING_SPEED = 100;
    logPrint(INFO, INFO, "Winder::operateUnwind\n");
    float speedRpm = speedMetersPerMinute * 1000 / _hardC.motors.puller.mmPerRotation;

    _stepperSpool->switchModeOff();
    _stepperFerrari->movePosition(-FERRARI_POSITIONING_SPEED, _softC.ferrari_max - _hardC.motors.ferrariOffset);
    _stepperPuller->moveRotate(speedRpm);
    _currentWinderOperation = OPERATE_UNWINDING;
}

void operateWind(float speedMetersPerMinute) {
    float speedRpm = speedMetersPerMinute * 1000 / _hardC.motors.puller.mmPerRotation;
    uint8_t DESIRED_SPOOL_LOAD = 50;  // Desired spool load in percent (0-100)
    logPrint(INFO, INFO, "Winder::operateWind, start: %d, end: %d, speedRpmPuller: %.2f, desiredLoad: %d\n", _softC.ferrari_min,
             _softC.ferrari_max, speedRpm, DESIRED_SPOOL_LOAD);

    if (_currentWinderOperation == OPERATE_PULLING || _currentWinderOperation == OPERATE_WINDING) {
        // Keep rotating without interrupt
        _stepperPuller->adjustMoveSpeed(-speedRpm);
        if (_currentWinderOperation == OPERATE_WINDING) {
            _stepperSpool->adjustMoveSpeed(-speedRpm / 5);  // TODO: Hardcoded
        }
    } else {
        _stepperPuller->moveRotate(-speedRpm);
    }
    if (_currentWinderOperation != OPERATE_WINDING) {
        _stepperSpool->moveRotateWithLoadAdjust(-speedRpm / 5, 15);  // TODO: Hardcoded - DESIRED_SPOOL_LOAD?
        _stepperFerrari->moveOscillate(
            speedRpm, _softC.ferrari_max - _hardC.motors.ferrariOffset,
            _softC.ferrari_min - _hardC.motors.ferrariOffset);  // Oscillate, speed will be adjusted repeatedly in handleSpeedAdjust()
    }

    _currentWinderOperation = OPERATE_WINDING;
}

void operateCalibrate(uint16_t calibrationPosition) {
    const float FERRARI_POSITIONING_SPEED = 100;
    logPrint(INFO, INFO, "Winder::operateCalibrate\n");
    _stepperSpool->switchModeStandby();
    _stepperFerrari->movePosition(-FERRARI_POSITIONING_SPEED, calibrationPosition);
    _stepperPuller->switchModeStandby();
    _currentWinderOperation = OPERATE_STANDBY;  // TODO: Switch OPERATE_CALIBRATING ?
}

void operateStandby() {
    logPrint(INFO, INFO, "Winder::operateStandby\n");
    _stepperSpool->switchModeStandby();
    _stepperFerrari->switchModeStandby();
    _stepperPuller->switchModeStandby();
    _currentWinderOperation = OPERATE_STANDBY;
}

void operateOff() {
    logPrint(INFO, INFO, "Winder::operateOff\n");
    _stepperSpool->switchModeOff();
    _stepperFerrari->switchModeOff();
    _stepperPuller->switchModeOff();
    _currentWinderOperation = OPERATE_OFF;
}

const char* winderOperationToString(winderOperation_e winderOperation) {
    switch (winderOperation) {
        case OPERATE_OFF:
            return "standby";  // TODO: Legacy, adjust and update webfrontend to something more intuitive
        case OPERATE_STANDBY:
            return "power";  // TODO: Legacy, adjust and update webfrontend to something more intuitive
        case OPERATE_PULLING:
            return "pulling";
        case OPERATE_WINDING:
            return "winding";
        case OPERATE_UNWINDING:
            return "unwinding";
        case OPERATE_CALIBRATING:
            return "calibrating";
    };
    return "error";
}
