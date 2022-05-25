#include "HeatController.h"

#include <Arduino.h>

#include "../../logger/logging.h"

#include "../../validator/McValidator.h"

/**
 * @brief Calculates the difference of two millis()-timestamps
 * 
 * @param timestampStart Timestamp of first measurement
 * @param timestampEnd  Timestamp of second measurement
 * @return uint64_t Timedifference between measurements in milliseconds
 */
uint64_t timeDifference(uint64_t timestampStart, uint64_t timestampEnd){
    return timestampEnd - timestampStart; // TODO: Handle overflow, though not that important unless you plan to have the machine running > 50 days straight
}

HeatController::HeatController(heaterControllerParameters_s config){
    _config = config;
    setTargetTemperature(config.targetTemp);

    init();
    uint64_t now = millis();
    _timestampSensorPrepare = 0;
    _timestampSensorRead = now;
    _timestampHeatingChange = 0;
    _heatingState = false;
}

void HeatController::setTargetTemperature(float temperature){
    _config.targetTemp = temperature > 350 ? 350 : temperature;
}

void HeatController::start(){
    if(!isReady()) return;
    _controllerState = ACTIVE;
}

void HeatController::stop(){
    if(!isReady()) return;
    _controllerState = STANDBY;
    activateHeater(false, true);
}

bool HeatController::isActive(){
    return _controllerState == ACTIVE;
}

bool HeatController::isReady(){
    return _controllerState != INVALID;
}

void HeatController::calculatePid(float currentTemperature, uint64_t currentTime, uint64_t previousTime){
    float PID_p = 0;
    float PID_i = 0;
    float PID_d = 0;
    float elapsedTime = (float) currentTime - (float) previousTime / 1000; // Time since last read in s
    float PID_error = _config.targetTemp - currentTemperature;
    
    logPrint(_logging, INFO, "{id: %d, timeNow: %d, timePrev: %d, timeD: %d, tempNow: %.2f, tempTarget: %.2f, p: %.2f, i: %.2f, d: %.2f, , pidErr: %.2f, _pidPrevErr: %.2f, pidVal: %.2f}\n",
        _config.id, currentTime, previousTime, elapsedTime, currentTemperature, _config.targetTemp, \
        PID_p, PID_i, PID_d, PID_error, _pidPreviousError, _pidValue);

    // Calulate PID
    PID_p = PID_CONST_P * PID_error;
    if(-3 < PID_error || PID_error < 3){ // Calculate I in a range of +-3   // TODO: Hardcoded 3?
        PID_i = PID_i + (PID_CONST_I * PID_error);
    }
    PID_d = PID_CONST_D * ((PID_error - _pidPreviousError) / elapsedTime);
    _pidValue = PID_p + PID_i + PID_d;

    // Adjust pid-values to fit the activation-cycles of the heater
    if(_pidValue < 0) _pidValue = 0;
    else if(_pidValue < HEATER_ACTIVATION_MINIMAL_DELAY_MS) _pidValue = HEATER_ACTIVATION_MINIMAL_DELAY_MS; // don't activate the heating element for less than a second to avoid unnecessary wear
    if(_pidValue > HEATER_ACTIVATION_CYCLE_MS) _pidValue = HEATER_ACTIVATION_CYCLE_MS;
    else if(_pidValue > HEATER_ACTIVATION_CYCLE_MS - HEATER_ACTIVATION_MINIMAL_DELAY_MS) _pidValue = HEATER_ACTIVATION_CYCLE_MS - HEATER_ACTIVATION_MINIMAL_DELAY_MS;

    _pidPreviousError = PID_error;
}

void HeatController::activateHeater(bool active, bool updateStates){
    digitalWrite(_config.pinHeat, active ? HIGH : LOW);
    if(updateStates){
        _heatingState = active;
        _timestampHeatingChange = millis();
    }
}

void HeatController::handle(){
    if(!isReady() || _controllerState != ACTIVE) return;
    uint64_t now = millis();

    // Read sensor, if it was prepared at least 1ms ago
    if(_timestampSensorPrepare >= _timestampSensorRead && timeDifference(_timestampSensorPrepare, now) >= 1){
        // Measure temperature and redo calculations
        float currentTemperature = readSensor();
        calculatePid(currentTemperature, now, _timestampSensorRead);
        _timestampSensorRead = now; // safe readtime AFTER, since the algorithm needs the old value to calculate the difference
    }

    // Prepare sensor for next measurement
    if(_timestampSensorPrepare < _timestampSensorRead && timeDifference(_timestampSensorRead, now) > DELAY_MEASUREMENTS_MS){
        prepareSensor();
        _timestampSensorPrepare = now;
    }

    // Adjust heating according to values calculated by PID
    if(_heatingState && timeDifference(_timestampHeatingChange, now)>_pidValue && _pidValue<HEATER_ACTIVATION_CYCLE_MS){
        logPrint(_logging, WARNING, "{id: %d, time: %d, action=\"stop heat\"}\n", _config.id, now);
        activateHeater(false, true);
    }
    else if(!_heatingState && timeDifference(_timestampHeatingChange, now)>(HEATER_ACTIVATION_CYCLE_MS-_pidValue) && _pidValue>0){
        logPrint(_logging, WARNING, "{id: %d, time: %d, action=\"start heat\"}\n", _config.id, now);
        activateHeater(true, true);
    }
}

void HeatController::init(){
    _controllerState = INVALID;
    if(!_mcValidator.isDigitalPinValid(_config.pinHeat)) return;
    if(!_mcValidator.isDigitalPinValid(_config.pinSensorSo)) return;
    if(!_mcValidator.isDigitalPinValid(_config.pinSensorCs)) return;
    if(!_mcValidator.isDigitalPinValid(_config.pinSensorSck)) return;
    pinMode(_config.pinHeat, OUTPUT);
    pinMode(_config.pinSensorCs, OUTPUT);
    pinMode(_config.pinSensorSo, INPUT);
    pinMode(_config.pinSensorSck, OUTPUT);
    _controllerState = STANDBY;
}

void HeatController::prepareSensor(){
    if(_controllerState == INVALID) return;
    digitalWrite(_config.pinSensorCs, LOW);
}

float HeatController::readSensor(){
    if(_controllerState == INVALID) return NAN;

    // Read in 16 bits,
    //  15    = 0 always
    //  14..2 = 0.25 degree counts MSB First
    //  2     = 1 if thermocouple is open circuit
    //  1..0  = uninteresting status
    uint16_t tempBits = shiftIn(_config.pinSensorSo, _config.pinSensorSck, MSBFIRST);
    tempBits <<= 8;
    tempBits |= shiftIn(_config.pinSensorSo, _config.pinSensorSck, MSBFIRST);
    digitalWrite(_config.pinSensorCs, HIGH);

    // Bit 2 indicates if the thermocouple is disconnected
    if (tempBits & 0x4) return NAN;

    // The lower three bits (0,1,2) are discarded status bits
    tempBits >>= 3;

    // The remaining bits are the number of 0.25 degree (C) counts
    return tempBits * 0.25;
}

void HeatController::setDebuggingLevel(loggingLevel_e level){
    _logging = level;
}