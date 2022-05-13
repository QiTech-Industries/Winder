#pragma once

// Related
// System / External
// Selfmade
#include "controller/stepper/Stepper.h"
// Project
#include <WinderWifi.h>
#include <Webserver.h>
#include <QiUpdater.h>

/**
 * @brief State / Operation-mode of a winder-machine
 * 
 */
enum machineWinderOperation_e{
    OPERATE_OFF, // Motors are unpowered, no operations
    OPERATE_STANDBY, // Motors are powered, no operations
    OPERATE_PULLING, // Puller-motor is active
    OPERATE_WINDING, // Winding filament on spools with adjusted pulling-strength
    OPERATE_UNWINDING, // Unwinding filament from the spool
    OPERATE_CALIBRATING // Interactive calibration of the ferrari end-positions // TODO: Unused due to redesign, remove?
};

/**
 * @brief Converts an operation-identifier into its text-representation
 * 
 * @param machineOperation operation to be converted
 * @return "error" on error, representation otherwise
 */
const char* machineWinderOperationToString(machineWinderOperation_e machineOperation);

/**
 * @brief Convert a speed in rotations per minute to mm per meters per minute
 * 
 * @param speedRpm speed to be converted in rpm
 * @param mmPerRotation mm per rotation
 * @return uint32_t 
 */
uint32_t speedRpmToMpm(float speedRpm, float mmPerRotation);

/**
 * @brief Manages the operation of QiTech's JarvisWinder machine, that is used to wind filaments on spools, offering a web-ui and wifi to the user
 */
class QiMachineWinder{
    private:
        const float FERRARI_POSITIONING_SPEED = 200;  // Positioning speed of the ferrari in rotations per minute
        const float MAX_SPEED_MPM = 100; // Maximum rotation speed in meters per minute (depending on the spool-diameter), <= 0 if none - TODO: Adjust to realistic limit

        // General configuration
        configurationMachineWinderSoftware_s _configurationSoft;
        configurationMachineWinderHardware_s _configurationHard;

        // Network
        QiUpdater _updater;
        WinderWifi _wifi;
        Webserver _server;

        // Hardware
        // TODO: Reconsider whether to use as pointers or references, adjusting deconstructor accordingly
        FastAccelStepperEngine* _engine; // Engine used for initialisation and coordination of motors - TODO: Set by parameter and create externally?
        Stepper* _stepperSpool; // Motor used for turning the spool for spooling the filament with a load-adjusted even strength
        Stepper* _stepperFerrari; // Motor used for positioning of the filament on the spool
        Stepper* _stepperPuller; // Motor used for pulling the filament

        // Operation
        machineWinderOperation_e _currentWinderOperation = OPERATE_OFF; // Current state/operation of the winder
        float _pullSpeedRpm; // Speed of puller in rotations per minute

        // Timining / Threads
        TimerForMethods<QiMachineWinder> *_timedReporter; // Timer for repeatedly reporting the current winder state to the user via web
        TimerForMethods<QiMachineWinder> *_timedSpeedAdjuster; // Timer for adjusting the speed of the different motors according to the load

        /**
         * @brief Nonfunctional output of a logo on the debugging console
         */
        void printBanner();

        /**
         * @brief Check a speed value for validity and adjust as needed
         * 
         * @param speedMetersPerMinute rotation speed in meters per minute (depending on the spool-diameter)
         * @return float new fixed value
         */
        float fixSpeedValue(float speedMetersPerMinute);

        /**
         * @brief Calculate the speed-ratio between spool and ferrari depending on the spoolsize
         * 
         * @return float ratio
         */
        float calculateSpoolToFerrariSpeedRation();

    public:
        // Constructor
        QiMachineWinder();

        /**
         * @brief Initialise the machine, starting network related controllers and entering standby
         */
        void init(configurationMachineWinder_s conf);

        /**
         * @brief Handle the current operation and states and the transitions between. To be repeatedly called externally
         */
        void handle();

        /**
         * @brief Enter pull-mode, which most notably is used to change spools
         * 
         * @param speedMetersPerMinute rotation speed in meters per minute (depending on the spool-diameter)
         */
        void operatePull(float speedMetersPerMinute);

        /**
         * @brief Enter wind-mode, used for putting filament on a spool
         * 
         * @param speedMetersPerMinute rotation speed in meters per minute (depending on the spool-diameter)
         */
        void operateWind(float speedMetersPerMinute);

        /**
         * @brief Enter unwind-mode, used to unwind filament from a spool
         * 
         * @param speedMetersPerMinute rotation speed in meters per minute (depending on the spool-diameter)
         */
        void operateUnwind(float speedMetersPerMinute);

        /**
         * @brief Enter calibration-mode, moving the ferrari instantly according to changes in endpositions, allowing for a precise calibration
         */
        void operateCalibrate();

        /**
         * @brief Enter standby, doing nothing but keeping the motors powered
         */
        void operateStandby();

        /**
         * @brief Do nothing and un-power motors
         */
        void operateOff();

        /**
         * @brief Help with calibration of ferrari by positioning it to the specified position, allowing for precise adjustments
         * 
         * @param calibrationPosition position to move ferrari to 
         */
        void operateCalibrate(uint16_t calibrationPosition);

        /**
         * @brief Change the current speed without changing or interrupting the operation mode
         * 
         * @param speedMetersPerMinute New rotation speed in meters per minute (depending on the spool-diameter)
         */
        void adjustSpeed(float speedMetersPerMinute);

        /**
         * @brief Change the start- and end-positions for the ferrari-oscillation without changing or interrupting the operation mode
         * 
         * @param positionStart Startposition
         * @param positionEnd Endposition
         */
        void adjustOscillationPositions(uint16_t positionStart, uint16_t positionEnd);

        /**
         * @brief Report the current winder state to the user via web
         */
        void handleStatusReport();

        /**
         * @brief Repeatedly called for adjusting the speed of the different motors according to the load
         */
        void handleSpeedAdjust();

        /**
         * @brief Helper-method for interactive debugging
         * 
         * @param cmd command-id, which equals the corresponding key having been pressed
         */
        void executeDebugCommand(char cmd);

        // Getter-Method
        machineWinderOperation_e getCurrentMode();

        // Getter-method
        configurationMachineWinderSoftware_s& getConfigurationSoft();
};
