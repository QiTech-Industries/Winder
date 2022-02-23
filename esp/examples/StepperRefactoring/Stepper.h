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
    mode_e mode;        // stepper operation mode
    float rpm;          // Steper Rotations per minute
    uint8_t load;       // Stepper load in %
    int32_t position1;  // Left end postion
    int32_t position2;  // Right end position
};

/**
 * @brief current stepper status
 *
 */
struct status_s {
    float rpm;         // Steper Rotations per minute
    uint8_t load;      // Stepper load in %
    int32_t position;  // stepper position
};

// TODO: Inherit from BaseController
class Stepper {
   private:
    TMC2130Stepper *_driver;
    FastAccelStepperEngine _engine = FastAccelStepperEngine();
    FastAccelStepper *_stepper = NULL;

    // TODO: change constants to uppercase
    const uint8_t _stallValue =
        8;  // stall config needed for intialization of TMCStepper
    // TODO: make acceleration configurable
    const uint16_t _acceleration = 1000;  // default sepper acceleration

    bool _homed = false;              // is stepper homed flag
    bool _startSpeedReached = false;  // start speed for load detection
    uint32_t _microstepsPerRotation;  // count of step signal to be sent for one
                                      // rotation
    stepper_s _config;                // stepper hard config
    status_s _current;                // average motor operating current
    const recipe_s _defaultRecipe = {
        .mode = OFF, .rpm = 0, .load = 0, .position1 = 0, .position2 = 0};
    recipe_s _currentRecipe = _defaultRecipe;  // current operation mode
    recipe_s _targetRecipe = _defaultRecipe;   // target operation mode

    /**
     * @brief Check if stepper is moving or rotating
     *
     * @return true Stepper is moving
     * @return false Stepper is off or in standby
     */
    bool isMoving();

    /**
     * @brief Check wether end condition of recipe is met
     *
     * @return true Stepper Homed/Positioned/at left or right stop
     * @return false Stepper still moving or in standby/off
     */
    bool isRecipeFinished();

    /**
     * @brief Check wether target mode needs homing
     *
     * @param targetMode desired mode stepper should switch to
     * @param currentMode current mode of operation
     * @return true targetMode needs position accuracy but stepper is not homed
     * yet
     * @return false mode does not require accuracy or
     */
    bool needsHome(mode_e targetMode, mode_e currentMode);

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
     * @brief Switches the current recipe based on transition conditions e.g.
     * HOME->POSITION, POSITION->STANDBY
     *
     * @return true Mode has switched
     * @return false Mode stays constant
     */
    // TODO: fix conditions for mode switching
    bool switchCurrentRecipeMode();

    /**
     * @brief Execute current recipe according to parameters set in
     * _currentRecipe
     *
     */
    void startRecipe();

    /**
     * @brief Stop the stepper immediatly but keep position (emergency stop)
     *
     */
    void forceStop();

    /**
     * @brief Update recipe according to current load or speed
     *
     */
    void tuneCurrentRecipe();

    /**
     * @brief Log StepperID, Mode, Speed, Load to Serial
     *
     */
    void printStatus();

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
    void off();

    /**
     * @brief Feed current to stepper without moving it
     *
     */
    void standby();

    /**
     * @brief Rotate stepper forever with set rpm
     *
     * @param rpm target stepper speed in rotationsPerMinute
     * negative values change direction
     */
    void rotate(float rpm);

    /**
     * @brief Rotate stepper while keeping measured load at setpoint
     *
     * @param startSpeed Speed at which load detection reliably works
     * @param desiredLoad Target load in %
     */
    void adjustSpeedToLoad(float startSpeed, uint8_t desiredLoad);

    /**
     * @brief Move stepper with rpm until load reaches 100%
     *
     * @param rpm target stepper speed in rotationsPerMinute
     * negative values change direction
     */
    void home(float rpm);

    /**
     * @brief Move stepper to target position
     *
     * @param rpm movement speed in rotations per minute
     * @param position target postion in mm absolut
     */
    void position(float rpm, int32_t position);

    /**
     * @brief Move Stepper between two positions
     *
     * @param rpm target stepper speed in rotationsPerMinute negative values
     * change direction
     * @param leftPos left end position
     * @param rightPos right end position
     */
    void oscillate(float rpm, int32_t leftPos, int32_t rightPos);

    /**
     * @brief Called from loop(), handles calls to time dependend stepper
     * methods
     *
     */

    // TODO: add debugging method
    void handle();
};