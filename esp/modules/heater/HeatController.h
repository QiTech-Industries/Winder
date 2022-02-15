/**
 * @brief Controller of the Heating-Module with integrated temperature sensor
 * 
 * Turns a heater on and off, trying to maintain a define the specified temperature as stable as possible, using a PID-algorithm
 * TODO: Check code for potential issues on time-overflow
 */
class HeatController{
    private:
        unsigned int _id; // Controller id
        double _targetTemperature; // Target temperature in degree celsius

        // Tweakable configuration parameters
        const int _heater_activation_cycle = 3000; // Duration for simulating pwm-activation of the heater in m for the pid-temperature-regulation
        const int _heater_activation_minimal_trigger_delay = 1000; // Minimal delay before the heater can change states again, to prevent wear on the relays
        const float PID_CONST_P = 9.1; // Adjustable parameter of the PID-algorithm
        const float PID_CONST_I = 0.3; // Adjustable parameter of the PID-algorithm
        const float PID_CONST_D = 1.8; // Adjustable parameter of the PID-algorithm
        const unsigned int _delayMeasurements = 250; // Delay between temperature-measurements in ms

        // Pins
        uint8_t _pin_heat; // Pin-number of the Pin of the SSR-40-relay
        uint8_t _pin_sensor_so; // Pin-number of the SO-Pin of the MAX6675-sensor
        uint8_t _pin_sensor_cs; // Pin-number of the CS-Pin of the MAX6675-sensor
        uint8_t _pin_sensor_sck; // Pin-number of the SCK-Pin of the MAX6675-sensor

        // States
        enum module_state_e {INVALID, STANDBY, ACTIVE};
        module_state_e controllerState = INVALID;
        boolean _heatingState; // State of heating module, true = active(hot), false = inactive
        unsigned long _timestampSensorPrepare; // millis()-timestamp since last preparation of temperature measurement
        unsigned long _timestampSensorRead; // millis()-timestamp since last temperature measurement
        unsigned long _timestampHeatingChange = 0; // millis()-timestamp of last change(activation / deactivation) of the heating module

        // Variables of the pid-algorithm
        float _pid_previous_error = 0; // Parameter of the pid-algorithm
        float PID_value = 0; // Result of the pid-algorithm

        /**
         * @brief Checks whether a pin is valid and exists
         * TODO: Move to helperclass?
         * 
         * @param pin Pin-number
         * @return true pin exists
         * @return false pin does not exist
         */
        bool isDigitalPinValid(uint8_t);

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
        void calculatePid(double, unsigned long, unsigned long);

        /**
         * @brief Prepare the sensor to be read after a short delay (1ms)
         */
        void prepareSensor();

        /**
         * @brief Reads data from the MAX6675-sensor. Requires the sensor to be previously prepared with prepareSensor() with >= 1ms
         * @see Copypasta see https://electronoobs.com/eng_arduino_tut24.php
         * @return double Temperature in celius - can be NAN if not initialised or if thermocouple is disconnected // TODO: Include case in unit-test
         */
        double readSensor();
    
    public:
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
        void handleStates();

        /**
         * @brief Setter-method for the target temperature to be maintained. Does NOT (de-)activate the heating module
         * 
         * @param temperature target temperature in degree celsius, limited to 350 C
         */
        void setTargetTemperature(double);

        /**
         * @brief (De-)Activate the heating module
         * 
         * @param active true = start/continue heating, false = stop heating
         * @param updateStates true = update internal states, false = do not update internal states
         */
        void activateHeater(bool, bool);
    
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
    HeatController (unsigned int, double, uint8_t, uint8_t, uint8_t, uint8_t);
};