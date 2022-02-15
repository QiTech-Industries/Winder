#include <Arduino.h>
#include <HeatController.h>

#define DEBUG_HEATER 1 // TODO - TEMP
#ifdef DEBUG_HEATER
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x...)
#endif


/**
 * @brief Calculates the difference of two millis()-timestamps
 * 
 * @param timestampStart Timestamp of first measurement
 * @param timestampEnd  Timestamp of second measurement
 * @return unsigned long Timedifference between measurements in milliseconds
 */
unsigned long timeDifference(unsigned long timestampStart, unsigned long timestampEnd){
    return timestampEnd - timestampStart; // TODO: Handle overflow, though not that important unless you plan to have the machine running > 50 days straight
}

HeatController::HeatController (unsigned int id, double targetTemp, uint8_t pin_heat, uint8_t pin_sensor_so, uint8_t pin_sensor_cs, uint8_t pin_sensor_sck) : _id(id), _pin_heat(pin_heat), _pin_sensor_so(pin_sensor_so), _pin_sensor_cs(pin_sensor_cs), _pin_sensor_sck(pin_sensor_sck){
    initPins();
    unsigned long now = millis();
    _timestampSensorPrepare = 0;
    _timestampSensorRead = now;
    _timestampHeatingChange = 0;
    _heatingState = false;
    setTargetTemperature(targetTemp);
}

void HeatController::setTargetTemperature(double temperature){
    _targetTemperature = temperature > 350 ? 350 : temperature;
}


void HeatController::start(){
    if(!isReady()) return;
    controllerState = ACTIVE;
}

void HeatController::stop(){
    if(!isReady()) return;
    controllerState = STANDBY;
    activateHeater(false, true);
}

bool HeatController::isActive(){
    return controllerState == ACTIVE;
}

bool HeatController::isReady(){
    return controllerState != INVALID;
}

void HeatController::calculatePid(double currentTemperature, unsigned long currentTime, unsigned long previousTime){
    float PID_p = 0;
    float PID_i = 0;
    float PID_d = 0;
    float elapsedTime = (double) currentTime - (double) previousTime / 1000; // Time since last read in s
    float PID_error = _targetTemperature - currentTemperature;
    
    DEBUG_PRINTF("{id: %d, timeNow: %d, timePrev: %d, timeD: %.3f, tempNow: %.2f, tempTarget: %.2f, p: %.2f, i: %.2f, d: %.2f, , pidErr: %.2f, _pidPrevErr: %.2f, pidVal: %.2f}\n",
        _id, currentTime, previousTime, elapsedTime, currentTemperature, _targetTemperature, \
        PID_p, PID_i, PID_d, PID_error, _pid_previous_error, PID_value);

    // Calulate PID
    PID_p = PID_CONST_P * PID_error;
    if(-3 < PID_error || PID_error < 3){ // Calculate I in a range of +-3   // TODO: Hardcoded 3?
        PID_i = PID_i + (PID_CONST_I * PID_error);
    }
    PID_d = PID_CONST_D * ((PID_error - _pid_previous_error) / elapsedTime);
    PID_value = PID_p + PID_i + PID_d;

    // Adjust pid-values to fit the activation-cycles of the heater
    if(PID_value < 0) PID_value = 0;
    else if(PID_value < _heater_activation_minimal_trigger_delay) PID_value = _heater_activation_minimal_trigger_delay; // don't activate the heating element for less than a second to avoid unnecessary wear
    if(PID_value > _heater_activation_cycle) PID_value = _heater_activation_cycle;
    else if(PID_value > _heater_activation_cycle - _heater_activation_minimal_trigger_delay) PID_value = _heater_activation_cycle - _heater_activation_minimal_trigger_delay;

    _pid_previous_error = PID_error;
}

void HeatController::activateHeater(bool active, bool updateStates){
    digitalWrite(_pin_heat, active ? HIGH : LOW);
    if(updateStates){
        _heatingState = active;
        _timestampHeatingChange = millis();
    }
}

void HeatController::handleStates(){
    if(!isReady() || controllerState != ACTIVE) return;
    unsigned long now = millis();

    // Read sensor, if it was prepared at least 1ms ago
    if(_timestampSensorPrepare >= _timestampSensorRead && timeDifference(_timestampSensorPrepare, now) >= 1){
        // Measure temperature and redo calculations
        double currentTemperature = readSensor();
        calculatePid(currentTemperature, now, _timestampSensorRead);
        _timestampSensorRead = now; // safe readtime AFTER, since the algorithm needs the old value to calculate the difference
    }

    // Prepare sensor for next measurement
    if(_timestampSensorPrepare < _timestampSensorRead && timeDifference(_timestampSensorRead, now) > _delayMeasurements){
        prepareSensor();
        _timestampSensorPrepare = now;
    }

    // Adjust heating according to values calculated by PID
    if(_heatingState && timeDifference(_timestampHeatingChange, now)>PID_value && PID_value<_heater_activation_cycle){
        DEBUG_PRINTF("{id: %d, time: %d, action=\"stop heat\"}\n", _id, now);
        activateHeater(false, true);
    }
    else if(!_heatingState && timeDifference(_timestampHeatingChange, now)>(_heater_activation_cycle-PID_value) && PID_value>0){
        DEBUG_PRINTF("{id: %d, time: %d, action=\"start heat\"}\n", _id, now);
        activateHeater(true, true);
    }
}

bool HeatController::isDigitalPinValid(uint8_t pin){
    switch(pin){
        // SPI-pins
            case 18:
            case 19:
            case 23:
            case 05:
        // Digital pins
            case 13:
            case 12:
            case 14:
            case 27:
            case 16:
            case 17:
            case 25:
            case 26:
                return true;
        // Analog pins:
            case 2:
            case 4:
            case 36:
            case 34:
            case 38:
            case 39:
            default:
                return false;
    }
}

void HeatController::initPins(){
    controllerState = INVALID;
    if(!isDigitalPinValid(_pin_heat)) return;
    if(!isDigitalPinValid(_pin_sensor_so)) return;
    if(!isDigitalPinValid(_pin_sensor_cs)) return;
    if(!isDigitalPinValid(_pin_sensor_sck)) return;
    pinMode(_pin_heat, OUTPUT);
    pinMode(_pin_sensor_cs, OUTPUT);
    pinMode(_pin_sensor_so, INPUT);
    pinMode(_pin_sensor_sck, OUTPUT);
    controllerState = STANDBY;
}

void HeatController::prepareSensor(){
    if(controllerState == INVALID) return;
    digitalWrite(_pin_sensor_cs, LOW);
}

double HeatController::readSensor(){
    if(controllerState == INVALID) return NAN;

    // Read in 16 bits,
    //  15    = 0 always
    //  14..2 = 0.25 degree counts MSB First
    //  2     = 1 if thermocouple is open circuit
    //  1..0  = uninteresting status
    uint16_t tempBits = shiftIn(_pin_sensor_so, _pin_sensor_sck, MSBFIRST);
    tempBits <<= 8;
    tempBits |= shiftIn(_pin_sensor_so, _pin_sensor_sck, MSBFIRST);
    digitalWrite(_pin_sensor_cs, HIGH);

    // Bit 2 indicates if the thermocouple is disconnected
    if (tempBits & 0x4) return NAN;

    // The lower three bits (0,1,2) are discarded status bits
    tempBits >>= 3;

    // The remaining bits are the number of 0.25 degree (C) counts
    return tempBits * 0.25;
}