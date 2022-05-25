#pragma once

#include <Utils.h>
#include <controller/stepper/Stepper.h>
#include <machine/Configuration.h>
#include <networking/QiUpdater.h>
#include <networking/QiWebserver.h>
#include <networking/QiWifi.h>
#include <timer.h>
#include <timerManager.h>

/**
 * @brief read mpm parameter from JSON, validate it and convert it to rpm
 *
 * @param data JSON object containing mpm parameter at root level
 * @param mmPerRotation  millimeters motor moves per rotation
 * @return float valid speed in rpm
 */
float parseMpmToRpm(JsonObject data, float mmPerRotation);

/**
 * @brief Manages the operation of QiTech's JarvisWinder machine, that is used to wind filaments on spools, offering a web-ui and wifi to
 * the user
 */
class Winder {
   private:
    // General configuration
    winderSoftwareConfiguration_s _softC;
    winderHardwareConfiguration_s _hardC;

    // Hardware
    // TODO: Reconsider whether to use as pointers or references, adjusting deconstructor accordingly
    FastAccelStepperEngine* _engine;  // Engine used for initialisation and coordination of motors
    Stepper* _stepperSpool;           // Motor used for turning the spool for spooling the filament with a load-adjusted even strength
    Stepper* _stepperFerrari;         // Motor used for positioning of the filament on the spool
    Stepper* _stepperPuller;          // Motor used for pulling the filament

    // Operation
    winderOperation_e _currentOperation = OPERATE_OFF;  // Current state/operation of the winder
    float _pullSpeedRpm;                                // Speed of puller in rotations per minute

    // Timining / Threads
    TimerForMethods<Winder>* _timedReporter;       // Timer for repeatedly reporting the current winder state to the user via web
    TimerForMethods<Winder>* _timedSpeedAdjuster;  // Timer for adjusting the speed of the different motors according to the load

    /**
     * @brief Nonfunctional output of a logo on the debugging console
     */
    void printBanner();

    /**
     * @brief Calculate the speed-ratio between spool and ferrari depending on the spoolsize
     *
     * @return float ratio
     */
    float calculateSpoolToFerrariSpeedRation();

   public:
    // Constructor
    Winder();

    /**
     * @brief Initialise the machine, starting network related controllers and entering standby
     */
    void init(winderConfiguration_s conf);

    /**
     * @brief Handle the current operation and states and the transitions between. To be repeatedly called externally
     */
    void handle();

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

    // Getter-Method
    winderOperation_e getCurrentMode();

    // Getter-method
    winderSoftwareConfiguration_s& getConfigurationSoft();
};
