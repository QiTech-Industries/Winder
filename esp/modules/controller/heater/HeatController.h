#pragma once
#include "../../logger/logging.h"

#include "../BaseController.h"

struct heaterControllerParameters_s {
    uint16_t id; // Controller id
    float targetTemp; // Target temperature in degree celsius
    uint8_t pinHeat; // Pin-number of the Pin of the SSR-40-relay
    uint8_t pinSensorSo; // Pin-number of the SO-Pin of the MAX6675-sensor
    uint8_t pinSensorCs; // Pin-number of the CS-Pin of the MAX6675-sensor
    uint8_t pinSensorSck; // Pin-number of the SCK-Pin of the MAX6675-sensor
};

/**
 * @brief Controller of the Heating-Module with integrated temperature sensor
 * 
 * Turns a heater on and off, trying to maintain a define the specified temperature as stable as possible, using a PID-algorithm
 * TODO: Check code for potential issues on time-overflow
 */
class HeatController : BaseController{
    private:
        heaterControllerParameters_s _config; // Configuration parameters including pins
        loggingLevel_e _logging = NONE; // Logging level, no logging by default

        // Tweakable configuration parameters
        const uint16_t HEATER_ACTIVATION_CYCLE_MS = 3000; // Duration for simulating pwm-activation of the heater in m for the pid-temperature-regulation
        const uint16_t HEATER_ACTIVATION_MINIMAL_DELAY_MS = 1000; // Minimal delay before the heater can change states again, to prevent wear on the relays
        const float PID_CONST_P = 9.1; // Adjustable parameter of the PID-algorithm
        const float PID_CONST_I = 0.3; // Adjustable parameter of the PID-algorithm
        const float PID_CONST_D = 1.8; // Adjustable parameter of the PID-algorithm
        const uint16_t DELAY_MEASUREMENTS_MS = 250; // Delay between temperature-measurements in ms

        // States
        enum module_state_e {INVALID, STANDBY, ACTIVE};
        module_state_e _controllerState = INVALID;
        bool _heatingState; // State of heating module, true = active(hot), false = inactive
        uint64_t _timestampSensorPrepare; // millis()-timestamp since last preparation of temperature measurement
        uint64_t _timestampSensorRead; // millis()-timestamp since last temperature measurement
        uint64_t _timestampHeatingChange = 0; // millis()-timestamp of last change(activation / deactivation) of the heating module

        // Variables of the pid-algorithm
        float _pidPreviousError = 0; // Parameter of the pid-algorithm
        float _pidValue = 0; // Result of the pid-algorithm

        /**
         * @brief Initialises the controller, for example by setting pins
         */
        void init();

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
         * @param config Configuration parameters including pins to be used
         */
        HeatController (heaterControllerParameters_s config);

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