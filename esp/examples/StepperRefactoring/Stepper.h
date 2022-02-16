#ifndef STEPPER_H
#define STEPPER_H

#include <FastAccelStepper.h>
#include <TMCStepper.h>
#include <helpers.h>

using TMC2130_n::DRV_STATUS_t;

struct recipe_s {
    mode_e mode;
    float rpm;     // Rotations per minute
    uint8_t load;  // Stepper load in %
    int32_t position1;
    int32_t position2;
};

struct status_s {
    float rpm;
    uint8_t load;
    int32_t position;
};

class Stepper {
   private:
    TMC2130Stepper *_driver;
    FastAccelStepperEngine _engine = FastAccelStepperEngine();
    FastAccelStepper *_stepper = NULL;

    const uint8_t _stallValue = 8;
    const uint16_t _acceleration = 1000;

    bool _homed = false;
    uint16_t _debugInterval = 0;
    uint32_t _microstepsPerRotation;
    stepper_s _config;
    recipe_s _targetRecipe;
    recipe_s _currentRecipe;
    status_s _current;
    const recipe_s _defaultRecipe = {.mode = OFF,
                                     .rpm = 0,   // Rotations per minute
                                     .load = 0,  // Stepper load in %
                                     .position1 = 0,
                                     .position2 = 0};

    bool isMoving();

    void printStatus();

    uint16_t getCurrentStall();

    void updateStatus();

    void forceStop();

    bool needsHome(mode_e mode);

    bool isRecipeFinished();

    bool switchCurrentRecipeMode();

    void startRecipe();

    void tuneCurrentRecipe();

   public:
    Stepper(stepper_s config);

    // Synchronous methods
    void oscillate(float rpm, int32_t leftPos, int32_t rightPos);

    void position(float rpm, int32_t position);

    void rotate(float rpm);

    void adjustSpeedToLoad(float startSpeed, uint8_t desiredLoad);

    void home(float rpm);

    void standby();

    void off();

    void debug(uint16_t interval);

    void loop();
};

#endif