#pragma once

#include <FastAccelStepper.h>
#include <StepperTest.h>
#include <TMCStepper.h>

using TMC2130_n::DRV_STATUS_t;

/**
 * @brief stepper movement instructions, movements that can be scheduled
 *
 */
struct recipe_s {
    mode_e mode;        // Operation mode
    float rpm;          // Speed in rotations per minute
    uint8_t load;       // Motorload in %, 0 = no load, 100 = full load
    int32_t position1;  // Left end position
    int32_t position2;  // Right end position
};

/**
 * @brief Status of stepper
 *
 */
struct status_s {
    float rpm;         // Steper Rotations per minute
    uint8_t load;      // Stepper load in %, 0 = no load, 100 = full load
    int32_t position;  // stepper position
};

// TODO: Inherit from BaseController
class Stepper {
   private:
    // Drivers
    TMC2130Stepper *_driver;
    FastAccelStepperEngine _engine = FastAccelStepperEngine();
    FastAccelStepper *_stepper = NULL;

    // Hardcoded configuration
    const uint8_t DRIVER_STALL_VALUE = 8;  // stall config needed for intialization of TMCStepper
    // TODO: make acceleration configurable
    const uint16_t DEFAULT_ACCELERATION = 1000;  // default stepper acceleration
    const float HOMING_SPEED_RPM = 40; // Speed for homing in rotations per minute

    // Soft configuration
    bool _debugging = false; // Flag whether to print debugging information
    stepper_s _config; // Stepper configuration
    uint32_t _microstepsPerRotation; // Count of step signals to be sent for one rotation

    // Status
    bool _homed = false; // Flag whether the driver of the stepper has been homed yet
    status_s _stepperStatus; // Current status of stepper
    
    // Recipes aka commands aka operation modes
    const recipe_s _defaultRecipe = {
        .mode = OFF, .rpm = 0, .load = 0, .position1 = 0, .position2 = 0};
    recipe_s _currentRecipe = _defaultRecipe;  // current operation mode
    recipe_s _targetRecipe = _defaultRecipe;   // target operation mode
    bool _newCommand = false; // Flag whether a new command is waiting in _targetRecipe

    /**
     * @brief Check if stepper is moving or rotating
     *
     * @return true Stepper is moving
     * @return false Stepper is off or in standby
     */
    bool isMoving();

    /**
     * @brief Check whether end condition of recipe is met
     *
     * @return true Stepper Homed/Positioned/at left or right stop
     * @return false Stepper still moving or in standby/off
     */
    bool isRecipeFinished();

    /**
     * @brief Checks whether defined starting speed has been reached
     * 
     * @return true speed reached
     * @return false speed not reached
     */
    bool isStartSpeedReached();

    /**
     * @brief Check wether target mode needs homing
     *
     * @param targetMode desired mode stepper should switch to
     * @param currentMode current mode of operation
     * @return true targetMode needs position accuracy but stepper is not homed
     * yet
     * @return false mode does not require accuracy or already homed
     */
    bool checkNeedsHome(mode_e targetMode, mode_e currentMode);

    /**
     * @brief Extract current stall from DRV_STATUS_t struct
     *
     * @return uint16_t raw load 0...1023
     */
    uint16_t getCurrentStall();

    /**
     * @brief Update _current struct with current rpm, load, position
     *
     */
    void updateStatus();

    /**
     * @brief Make the motor run at a defined speed
     * 
     * @param speedRpm speed in rotations per minute. If 0 the motor will be powered but not moving. Positive / negative values determine the direction
     */
    void applySpeed(float speedRpm);

    /**
     * @brief Start executing a recipe
     *
     * @param recipe recipe to be executed
     */
    void startRecipe(recipe_s recipe);

    /**
     * @brief Stop the stepper immediatly but remember position (emergency stop)
     *
     */
    void forceStop();

    /**
     * @brief Update the motor speed according to the current load
     *
     */
    void adjustSpeedByLoad();

   public:
    Stepper();

    /**
     * @brief Initialize FastAccelStepper and TMCStepper with defaults Must be
     * calles from setup
     *
     * @param config Unchangable Stepper hard config
     */
    void init(stepper_s config);

    /**
     * @brief Disable motor drivers, sets stepper in free-spin
     *
     */
    void switchModeOff();

    /**
     * @brief Keeps the motor powered while eactivating any current movement commands
     *
     */
    void switchModeStandby();

    /**
     * @brief Start rotating stepper forever with constant speed
     * 
     * @param rpm target stepper speed in rotationsPerMinute
     * negative values change direction
     */
    void moveRotate(float rpm);

    /**
     * @brief Rotate stepper while keeping measured load at setpoint
     *
     * @param startSpeed Speed at which load detection reliably works
     * @param desiredLoad Target load in %
     */
    void moveRotateWithLoadAdjust(float startSpeed, uint8_t desiredLoad);

    /**
     * @brief Start moving stepper with rpm until load reaches 100%
     *
     * @param rpm target stepper speed in rotationsPerMinute
     * negative values change direction
     */
    void moveHome(float rpm);

    /**
     * @brief Start moving stepper to target position
     *
     * @param rpm movement speed in rotations per minute
     * @param position target postion in mm absolut
     */
    void movePosition(float rpm, int32_t position);

    /**
     * @brief Start moving stepper between two positions
     *
     * @param rpm target stepper speed in rotationsPerMinute negative values
     * change direction
     * @param leftPos left end position
     * @param rightPos right end position
     * @param directionLeft true = start by moving left, false = start by moving right
     */
    void moveOscillate(float rpm, int32_t leftPos, int32_t rightPos, bool directionLeft=true);

    /**
     * @brief Manages states and transitions, repeatedly called
     *
     */
    void handle();

    /**
     * @brief Print anoverview of stepper-related status details, used for debugging
     * TODO: Remove eventually since debug-related?
     *
     * @param verbous true=all details, false=short details
     */
    void printStatus(bool verbous);

    // Setter-method
    void setDebugging(bool debugging);
};