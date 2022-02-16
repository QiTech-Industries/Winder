#pragma once
#include <logging.h>

/**
 * @brief Controller of the Heating-Module with integrated temperature sensor
 * 
 * Turns a heater on and off, trying to maintain a define the specified temperature as stable as possible, using a PID-algorithm
 * TODO: Check code for potential issues on time-overflow
 */
class HeatController{
    private:
        uint16_t _id; // Controller id
        loggingLevel_e _logging = NONE; // Logging level, no logging by default
        float _targetTemperature; // Target temperature in degree celsius

        // Tweakable configuration parameters
        const uint16_t HEATER_ACTIVATION_CYCLE_MS = 3000; // Duration for simulating pwm-activation of the heater in m for the pid-temperature-regulation
        const uint16_t HEATER_ACTIVATION_MINIMAL_DELAY_MS = 1000; // Minimal delay before the heater can change states again, to prevent wear on the relays
        const float PID_CONST_P = 9.1; // Adjustable parameter of the PID-algorithm
        const float PID_CONST_I = 0.3; // Adjustable parameter of the PID-algorithm
        const float PID_CONST_D = 1.8; // Adjustable parameter of the PID-algorithm
        const uint16_t DELAY_MEASUREMENTS_MS = 250; // Delay between temperature-measurements in ms

        // Pins
        uint8_t _pinHeat; // Pin-number of the Pin of the SSR-40-relay
        uint8_t _pinSensorSo; // Pin-number of the SO-Pin of the MAX6675-sensor
        uint8_t _pinSensorCs; // Pin-number of the CS-Pin of the MAX6675-sensor
        uint8_t _pinSensorSck; // Pin-number of the SCK-Pin of the MAX6675-sensor

        // States
        enum module_state_e {INVALID, STANDBY, ACTIVE};
        module_state_e _controllerState = INVALID;
        boolean _heatingState; // State of heating module, true = active(hot), false = inactive
        uint64_t _timestampSensorPrepare; // millis()-timestamp since last preparation of temperature measurement
        uint64_t _timestampSensorRead; // millis()-timestamp since last temperature measurement
        uint64_t _timestampHeatingChange = 0; // millis()-timestamp of last change(activation / deactivation) of the heating module

        // Variables of the pid-algorithm
        float _pidPreviousError = 0; // Parameter of the pid-algorithm
        float _pidValue = 0; // Result of the pid-algorithm

        /**
         * @brief Checks whether a pin is valid and exists
         * TODO: Move to helperclass?
         * 
         * @param pin Pin-number
         * @return true pin exists
         * @return false pin does not exist
         */
        bool isDigitalPinValid(uint8_t pin);

        /**
         * @brief Initialises needed pins
         * TODO: Candidate for moving into abstract class
         * 
         * @return true success
         * @return false error, probably wrong pin-numbers
         */
        void initPins();

        /**
         * @brief Calculation of changes needed for continuos temperature regulation via PID-algorithm
         * 
         * @param currentTemperature current temperature-measurement in degree celsius
         * @param currentTime current timestamp in millis
         * @param previousTime timestamp of last PID-calculation in millis
         */
        void calculatePid(float currentTemperature, uint64_t currentTime, uint64_t previousTime);

        /**
         * @brief Prepare the sensor to be read after a short delay (1ms)
         */
        void prepareSensor();

        /**
         * @brief Reads data from the MAX6675-sensor. Requires the sensor to be previously prepared with prepareSensor() with >= 1ms
         * @see Copypasta see https://electronoobs.com/eng_arduino_tut24.php
         * @return float Temperature in celius - can be NAN if not initialised or if thermocouple is disconnected // TODO: Include case in unit-test
         */
        float readSensor();
    
    public:
        /**
         * @brief Constructor
         * 
         * @param id ID of the controller
         * @param targetTemp Target temperature in degree celsius, limited to 350 C
         * @param pin_heat Pin-number of the SSR-40-relay
         * @param pin_sensor_so Pin-number of the SO-Pin of the MAX6675-Temperatursensor
         * @param pin_sensor_cs Pin-number of the CS-Pin of the MAX6675-Temperatursensor
         * @param pin_sensor_sck Pin-number of the SCK-Pin of the MAX6675-Temperatursensor
         */
        HeatController (uint16_t id, float targetTemp, uint8_t pin_heat, uint8_t pin_sensor_so, uint8_t pin_sensor_cs, uint8_t pin_sensor_sck);

        /**
         * @brief Checks whether the Controller was succesfully initialised and can be used
         * 
         * @return true Ready
         * @return false Something went wrong
         */
        bool isReady();

        /**
         * @brief Checks whether controller is active (= maintaining a temperature)
         * 
         * @return true Controller is active
         * @return false Controller is either on standby or not ready, also see isReady()
         */
        bool isActive();

        /**
         * @brief Start Controller
         */
        void start();

        /**
         * @brief Stop Controller, deactivating the heater
         */
        void stop();

        /**
         * @brief Called repeatedly, handles states and changes, such as reading temperatures and (de-)activating the heating element
         */
        void handle();

        /**
         * @brief Setter-method for the target temperature to be maintained. Does NOT (de-)activate the heating module
         * 
         * @param temperature target temperature in degree celsius, limited to 350 C
         */
        void setTargetTemperature(float temperature);

        /**
         * @brief (De-)Activate the heating module
         * 
         * @param active true = start/continue heating, false = stop heating
         * @param updateStates true = update internal states, false = do not update internal states
         */
        void activateHeater(bool active, bool updateStates);

        /**
         * @brief Set the current level of logging
         * 
         * @param level new level
         */
        void setDebuggingLevel(loggingLevel_e level);
};