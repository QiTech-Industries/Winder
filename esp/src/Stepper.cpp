// TODO: Replace with proper debugging/logging
#ifdef DEBUG_WINDER
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x...)
#endif

#include <Stepper.h>

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

using TMC2130_n::DRV_STATUS_t; // TODO - Comment

void Stepper::next()
{
    if (!stopped && queue[queueCurrent - 1].cb)
    {
        queue[queueCurrent - 1].cb();
    }

    // If another command is available in queue
    if (queue.size() > queueCurrent)
    {
        queueItem_s item = queue[queueCurrent];

        // DEFAULT SETTINGS
        stepper->forceStopAndNewPosition(stepper->getCurrentPosition()); // Stop RAMP generator
        stopOnStall = false;                                             // StallGuard deactivated
        driver->toff(1);                                                 // Activate Stepper driver
        adjustSpeedToStall = false;                                      //
        stopped = false;
        stepper->setAcceleration(config.default_acceleration);

        // special case: jumping back in queue can not be handled in switch
        if (item.command == LOOP)
        {
            queueCurrent = queueCurrent - item.mm; // mm abused as argument count of loop (last x to repeat)
            item = queue[queueCurrent];
        }

        switch (item.command)
        {
        case ROTATE:
            rotate(item.rps, item.mm);
            break;
        case HOME:
            stopOnStall = true;
            isHomed = true;
            rotate(item.rps);
            break;
        case POSITION:
            position(item.rps, item.mm);
            break;
        case ADJUST:
            adjustSpeedToStall = true;
            rotate(item.rps);
            break;
        default:
            break;
        }

        queueCurrent++;
    }
    else
    {
        stopped = true;
    }
}

bool Stepper::moveFinished()
{
    // Calculate driver status and speed
    DRV_STATUS_t drv_status{0};
    drv_status.sr = driver->DRV_STATUS();
    uint32_t speed = getSpeedUs();
    float calcLow = 0.19424 * speed + 38.97870; // TODO: Comment hardcoded calulcation

    // Log driver errors for debugging
    char driverErrorMessage[5] = {'-','-','-','-',0};
    if (driver->drv_err())
    {
        if (drv_status.otpw) driverErrorMessage[0] = 'H'; // Warning Stepper Driver overheated
        if (drv_status.ot) driverErrorMessage[1] = 'X'; // Stepper shut down due to overheated driver
        if (drv_status.s2ga or drv_status.s2gb) driverErrorMessage[2] = 'S'; // Stepper shut down due to short circuit
        if (drv_status.ola or drv_status.olb) driverErrorMessage[3] = 'O'; // Stepper driver detected open load
    }
    if(isLogRelevant(_logging, INFO)){ // TODO: Debug/Logging
        DEBUG_PRINTF("{stepid: %d, err: '%s', time: %lu, drv: %d, spd: %.2f, cont: %i, mRun: %i, posTarg: %i, posNow: %i}\n", _id, driverErrorMessage, millis(), drv_status.sg_result, calcLow, stepper->isRunningContinuously(), stepper->isMotorRunning(), stepper->targetPos(), stepper->getCurrentPosition());
    }

    if (!stepper->isMotorRunning())
    { // Motor not running - nothing moving
        return true;
    }
    else if (stopOnStall)
    { // Motor stalled, we hit something
        if(isLogRelevant(_logging, INFO)) DEBUG_PRINTF("{stepid: %d, ignore: %d, stepper: 'stoponstall'}", _id, ignore); // TODO: Debug/Logging

        // Check whether we are still ignoring the initial stall 
        if (ignore > 0){
            ignore--;
            return false;
        }

        // TODO: DEBUG_PRINTF("%u %f\n", drv_status.sg_result, calcLow);
        
        if (drv_status.sg_result < calcLow)
        { // Homing finished, stop and set position as new origin
            stepper->forceStopAndNewPosition(0);
            if(isLogRelevant(_logging, INFO))  DEBUG_PRINTF("{stepid: %d, stepper: 'homed'}", _id); // TODO: Debug/Logging
            return true;
        }
    }
    // Still moving without issues

    return false;
}

void Stepper::moveAdjust()
{
    // Retrieve status and speed
    float speed = (float) getSpeedUs(); // Motor speed in microseconds per motor step (smaller = faster)
    DRV_STATUS_t drv_status{0};
    drv_status.sr = driver->DRV_STATUS();

    // Adjust motor-speed according to current measured load
    if (adjustSpeedToStall)
    {
        // last speed value is dummy to prevent i+1 errors
        //float curve[]{660, 600, 580, 500.4, 465.6, 467.5, 401, 463.1, 371.3, 444.4, 374.2, 380.5, 364.7, 332.1, 387.9, 324.8, 323, 350, 356.4, 356.9, 309.5, 331, 324, 343.3, 316.4, 346.7, 311, 322, 300.2, 284.1, 326.3, 315.5, 320.5, 295.1, 282.7, 288, 315.1, 268.3, 265.5, 274.7};
        // default float curve[]{940,880,832.87,701.4,600.53,543.2,541.33,533.33,501.53,518.53,491.53,458.13,442.33,444.07,443.47,419.67,439.27,433.87,407.67,431.87,353.27,392.4,413.67,371,389.27,402.27,400.8,385.4,357.4,359,350.2,362.2,379.33,358.13,363.93,353,356.27,343.07,351.93,352.2};
        // raths float curve[]{1023, 878.33, 685.27, 594.67, 558.27, 526.67, 500.07, 502.53, 457.6, 501.07, 463.2, 423.67, 436.87, 420.6, 403.73, 409.13, 442.73, 420.07, 391.93, 403.73, 408.93, 392.13, 414.8, 399.6, 387.53, 413.13, 372.07, 366.53, 389.47, 391.13, 382.87, 356, 350.6, 380, 223.47, 380.4, 330, 378.4, 386.73, 380};
        // new Ferrari float curve[]{1014,890,763,680,632,619,588,600,568,589,541,515,507,495,506,489,481,485,478,478,442,465,461,463,458,455,449,449,442,438,444,438,439,431,427,422,423,415,416,415};
        // old ferrari
        float curve[]{ // TODO - comment / rename?
            989, 795, 680, 574, 533, 529, 477, 519, 452, 503,
            442, 435, 423, 398, 436, 392, 388, 405, 406, 406,
            364, 386, 380, 392, 374, 391, 367, 374, 358, 347,
            374, 365, 369, 351, 341, 343, 359, 328, 327, 332
        }; // Reference values for measured motor-loads
        float speeds[]{
            3619, 1809, 1206, 904, 723, 603, 517, 452, 402, 361,
            329, 301, 278, 258, 241, 226, 212, 201, 190, 180,
            172, 164, 157, 150, 144, 139, 134, 129, 124, 120,
            116, 113, 109, 106, 103, 100, 97, 95, 92, 90, 0
        }; // Reference values for speeds in microseconds per motor-step (smaller = faster) corresponding to the motor-loads

        for (int8_t i = 0; i < 40; i++)
        {
            if ((speeds[i] >= speed && speeds[i + 1] < speed) || speed >= speeds[0])
            {
                if (curve[i] > drv_status.sg_result)
                {
                    // Load too high, slow down
                    float newSpeed = min(speed * 1.1, speeds[0]);
                    DEBUG_PRINTF("{stepid: %d, act: 'spd-', spdOld: %.2f, spdNew: %.2f}\n", _id, speed, newSpeed); // TODO: Debug/Logging
                    setSpeedUs(newSpeed);
                }
                else
                {
                    // Load too low, speed up
                    float newSpeed = max(speed * 0.8, speeds[39]);
                    DEBUG_PRINTF("{stepid: %d, act: 'spd+', spdOld: %.2f, spdNew: %.2f}\n", _id, speed, newSpeed); // TODO: Debug/Logging
                    setSpeedUs(newSpeed);
                }
                break;
            }
        }
    }
}

void Stepper::rotate(float rps, uint16_t mm)
{
    Serial.print("ROTATE: "); // TODO: Debug/Logging
    Serial.println(rps);
    if (rps == 0)
    {
        stop();
        return;
    }

    stepper->setSpeedInHz((int)microsteps_per_rotation * abs(rps));

    if (mm == 0 && rps < 0)
    {
        stepper->runBackward();
    }
    else if (mm == 0 && rps > 0)
    {
        stepper->runForward();
    }
    else if (mm != 0 && rps < 0)
    {
        stepper->move((int)mm * microsteps_per_rotation / config.mm_per_rotation * -1);
    }
    else if (mm != 0 && rps > 0)
    {
        stepper->move((int)mm * microsteps_per_rotation / config.mm_per_rotation);
    }
}

void Stepper::stop(float seconds)
{
    if (seconds == 0)
    {
        stepper->forceStopAndNewPosition(stepper->getCurrentPosition());
        return;
    }

    int32_t speed = abs(stepper->getCurrentSpeedInUs());
    int32_t accel = 1000000;

    if (speed != 0)
    {
        accel = (int)accel / speed / seconds;
    }
    stepper->moveByAcceleration(-accel, false);
}

Stepper::Stepper() : timer(this, &Stepper::loop)
{
    SPI.begin();
}

void Stepper::setId(uint8_t id){
    _id = id;
}

void Stepper::init(stepper_s config)
{
    // https://www.trinamic.com/fileadmin/assets/Products/ICs_Documents/TMC2130_datasheet.pdf page 33

    driver = new TMC2130Stepper(config.pins.cs);

    driver->begin();
    engine.init();

    this->config = config;
    this->microsteps_per_rotation = config.steps_per_rotation * config.microsteps * config.gear_ratio;

    // DRIVER config
    driver->toff(1);
    driver->blank_time(5);
    driver->rms_current(config.max_current);
    driver->microsteps(config.microsteps);
    driver->sgt(config.stall);
    driver->sfilt(true);

    // StallGuard/Coolstep config
    driver->TCOOLTHRS(1000000);
    driver->semin(0);
    driver->semax(1);

    // FastAccelStepper config
    stepper = engine.stepperConnectToPin(config.pins.step);

    if (stepper)
    {
        stepper->setDirectionPin(config.pins.dir);
        stepper->setEnablePin(config.pins.en);
        stepper->setAutoEnable(true);
    }

    // Setup timer for loop (every 0,1s)
    timer.setInterval(100);
    timer.start();
}

void Stepper::setDebuggingLevel(loggingLevel_e level){
    _logging = level;
}
    
/*void Stepper::calibrateStall(float rpm)
{
    stepper->setAcceleration(100);
    stepper->applySpeedAcceleration();
    rotate(1);
    //add({.rps = 3, .mm = 0, ADJUST});
    for (uint8_t i = 0; i < 64; i++)
    {
        driver->sgt(7);
        delay(300); //Reliable results only after 300ms delay
        DRV_STATUS_t drv_status{0};
        drv_status.sr = driver->DRV_STATUS();
        Serial.println(drv_status.sg_result);
        if (drv_status.sg_result == 1023)
        {
            Serial.println("CALIBRATED STALL VALUE:");
            Serial.println(i);
            //break;
        }
    }
}*/

void Stepper::add(queueItem_s item)
{
    queue.push_back(item);
}

void Stepper::loop(uint8_t lastX)
{
    queueItem_s item{0, lastX, LOOP};
    add(item);
}

void Stepper::off()
{
    stop();
    driver->toff(0);
    queue.clear();
    isHomed = false;
    ignore = 25;
}

void Stepper::on()
{
    stop();
    driver->toff(1);
    queue.clear();
    queueCurrent = 0;
    stopped = true;
}

void Stepper::setSpeed(float rps)
{
    stepper->setSpeedInHz((int)microsteps_per_rotation * abs(rps));
    stepper->applySpeedAcceleration();
}

void Stepper::position(float rps, uint16_t position)
{
    // do not allow setting fixed end position if currently homing
    // update next command instead if it is of type POSITION
    if (stopOnStall)
    {
        if (queue[queueCurrent].command == POSITION)
        {
            queue[queueCurrent].mm = position;
        }
        return;
    }

    int32_t pos = position * microsteps_per_rotation / config.mm_per_rotation;
    stepper->setSpeedInHz((int)microsteps_per_rotation * abs(rps));
    stepper->moveTo(pos);
}

void Stepper::setSpeedUs(uint32_t speed)
{
    stepper->setSpeedInUs(speed);
    stepper->applySpeedAcceleration();
}

uint32_t Stepper::getSpeedUs()
{
    return abs(stepper->getCurrentSpeedInUs());
}

float_t Stepper::getSpeedRpm()
{
    return (60000000 / abs(stepper->getCurrentSpeedInUs()) / microsteps_per_rotation);
}

stepperStatus_s Stepper::getStatus()
{
    stepperStatus_s stats{0, 0, 0, 0, ""};
    DRV_STATUS_t drv_status{0};
    drv_status.sr = driver->DRV_STATUS();

    if (driver->drv_err())
    {
        if (drv_status.otpw)
        {
            stats.error = "Warning Stepper Driver overheated.";
        }
        else if (drv_status.ot)
        {
            stats.error = "Stepper shut down due to overheated driver.";
        }
        else if (drv_status.s2ga or drv_status.s2gb)
        {
            stats.error = "Stepper shut down due to short circuit.";
        }
        else if (drv_status.ola or drv_status.olb)
        {
            stats.error = "Stepper driver detected open load.";
        }
    }

    if (stepper->getCurrentSpeedInUs())
    {
        stats.rpm = (float)60000000 / stepper->getCurrentSpeedInUs() / microsteps_per_rotation;
    }

    stats.stall = drv_status.sg_result;
    stats.active = driver->toff() == 0 ? 0 : 1;
    stats.rotations = 0;

    return stats;
}

bool Stepper::getHomed()
{
    return isHomed;
}

void Stepper::loop()
{
    if (moveFinished())
    {
        next();
        return;
    }
    moveAdjust();
}