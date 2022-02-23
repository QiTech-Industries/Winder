#pragma once
// Built in Library inclusions
#include <Arduino.h> // Arduino standard library - remove include when logging-commands have been replaced?

// External Library inclusions
#include <timer.h> // Timer to schedule Commands
#include <LITTLEFS.h> // Store Webinterface in Filesystem
#include <TMCStepper.h> // Interface for TMC 2130 Stepper Driver
#include <FastAccelStepper.h> // Pulse generation for Stepper

// Custom Header file with globally available structs for data storage
#include <store.h>

#include "logger/logging.h"

// TODO - entfernen?
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

struct stepperStatus_s // State of the stepper-driver, used to display current status in webfrontend
{
    float rpm; // Speed in rotations per minute
    uint16_t stall; // TODO: Comment, result of drv_status.sg_result
    uint8_t active; // Current status of driver, result of driver->toff() - TODO: use bool instead?
    float rotations; // TODO: Comment, TODO: Remove: Only set to zero once, never read, what is the use of couting rotations anyway?
    String error; // Human-readable message for error-related states
};

/**
 * @brief TODO: Comment
 */
class Stepper
{
private:
    TMC2130Stepper *driver; // TODO: Comment
    FastAccelStepper *stepper = NULL; // TODO: Comment
    TimerForMethods<Stepper> timer; // TODO: Comment

    stepper_s config; // TODO: Comment
    struct queueItem_s // TODO: Comment
    {
        float rps; // Speed in rotations per seconds
        uint16_t mm; // TODO: Comment
        commands_s command; // Command to be executes by the stepper
        std::function<void()> cb; // Callback function TODO: Comment
    };

    std::vector<queueItem_s> queue; // TODO: Comment
    uint16_t queueCurrent; // TODO: Comment

    bool stopOnStall = false; // TODO: Comment
    bool adjustSpeedToStall = false; // TODO: Comment
    bool stopped = true; // TODO: Comment
    int ignore = 25; // TODO: Comment
    bool isHomed = false; // TODO: Comment
    uint8_t _id=0;
    loggingLevel_e _logging = NONE; // Logging level, no logging by default


    uint32_t microsteps_per_rotation; // TODO: Comment

    /**
     * @brief Get and start next command from queue
     */
    void next();

    /**
     * @brief TODO - Comment
     * 
     * TODO: Rename as name feels like isMoveFinished() despite having sideeffects
     * 
     * @return true There are no active movement commands
     * @return false There is an active movement command
     */
    bool moveFinished();

    /**
     * @brief TODO - Comment
     */
    void moveAdjust();

    /**
     * @brief TODO - Comment
     * 
     * TODO: positive values turn left or move ferrari away from motor
     * 
     * @param rps TODO - Comment
     * @param mm TODO - Comment
     */
    void rotate(float rps, uint16_t mm = 0);

    /**
     * @brief Stop the movement of the stepper
     * 
     * @param seconds time for decelerating in seconds, 0 = immediate - TODO: rename
     */
    void stop(float seconds = 0);

public:
    // Constructor
    Stepper();

    // Setter-method
    void setId(uint8_t id);

    /**
     * @brief Initialise the controller according to the configuration
     * 
     * TODO: starting the timers/intervals is not exactly intuitive
     * 
     * @param config configuration to be used
     */
    void init(stepper_s config);

    // Setter-method
    void setDebuggingLevel(loggingLevel_e level);

    // TODO - Comment
    // void calibrateStall(float rpm);

    /**
     * @brief TODO - Comment
     * 
     * QUEUE FUNCTIONALITY
     * 
     * @param item TODO - Comment
     */
    void add(queueItem_s item);

    /**
     * @brief TODO - Comment
     * 
     * QUEUE FUNCTIONALITY
     * 
     * @param item TODO - Comment
     */
    void loop(uint8_t lastX);

    /**
     * @brief TODO: Comment
     * 
     * TODO: STEPPER STATUS SETTERS
     */
    void off();

    /**
     * @brief TODO: Comment
     * 
     * TODO: STEPPER STATUS SETTERS
     */
    void on();

    /**
     * @brief TODO: Comment
     * 
     * TODO: STEPPER STATUS SETTERS
     * TODO: Not a setter, rename
     * 
     * @param rps TODO - Comment
     */
    void setSpeed(float rps);

    /**
     * @brief TODO: Comment
     * 
     * TODO: STEPPER STATUS SETTERS
     * TODO: rename
     * 
     * @param rps TODO: Comment
     * @param position TODO: Comment
     */
    void position(float rps, uint16_t position);

    /**
     * @brief TODO: Comment
     * 
     * TODO: Not a setter, rename
     * TODO: STEPPER STATUS SETTERS
     * 
     * @param speed TODO: Comment
     */
    void setSpeedUs(uint32_t speed);

    /**
     * @brief TODO - Comment
     * 
     * TODO: STEPPER STATUS GETTERS
     * 
     * @return uint32_t TODO - Comment
     */
    uint32_t getSpeedUs();

    /**
     * @brief TODO - Comment
     * 
     * TODO: STEPPER STATUS GETTERS
     * 
     * @return float_t 
     */
    float_t getSpeedRpm();

    /**
     * @brief TODO: Not a pure getter, rename?
     * 
     * TODO: STEPPER STATUS GETTERS
     * 
     * @return stepperStatus_s TODO - Comment
     */
    stepperStatus_s getStatus();

    // Getter-method - TODO: Rename
    // TODO: STEPPER STATUS GETTERS
    bool getHomed();

    /**
     * @brief Called repeatedly, handles states and changes
     * 
     * TODO: rename to handle() ?
     */
    void loop();
};