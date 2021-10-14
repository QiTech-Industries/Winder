#include <global.h>

using TMC2130_n::DRV_STATUS_t;

class Stepper
{
private:
    TMC2130Stepper *driver;
    FastAccelStepper *stepper = NULL;
    TimerForMethods<Stepper> timer;

    stepper_s config;
    struct queueItem_s
    {
        float rps;
        uint16_t mm;
        commands_s command;
        std::function<void()> cb;
    };

    struct status_s
    {
        float rpm;
        uint16_t stall;
        uint8_t active;
        float rotations;
        String error;
    };

    std::vector<queueItem_s> queue;
    uint16_t queueCurrent;

    bool stopOnStall = false;
    bool adjustSpeedToStall = false;
    bool stopped = true;
    int ignore = 25;
    uint32_t totalTicks = 0;
    uint32_t lastTicks = 0;
    bool isHomed = false;

    uint32_t microsteps_per_rotation;

    void next()
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
            stepper->forceStopAndNewPosition(stepper->getCurrentPosition()); //Stop RAMP generator
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

    bool moveFinished()
    {
        //DEBUG_PRINTF("STATS: %i %i %i %i \n", stepper->isRunningContinuously(), stepper->isMotorRunning(), stepper->isRampGeneratorActive(), stepper->isQueueEmpty());
        if (!stepper->isMotorRunning())
        {
            return true;
        }
        else if (stopOnStall)
        {
            ignore--;
            if (ignore > 0)
                return false;

            uint32_t speed = getSpeedUs();
            DRV_STATUS_t drv_status{0};
            drv_status.sr = driver->DRV_STATUS();

            float calcLow = 0.19424 * speed + 38.97870;

            DEBUG_PRINTF("%u %f\n", drv_status.sg_result, calcLow);

            if (drv_status.sg_result < calcLow)
            {
                stepper->forceStopAndNewPosition(0);
                Serial.println("HOMING FINSIHED");
                return true;
            }
        }

        return false;
    }

    void moveAdjust()
    {
        if (adjustSpeedToStall)
        {
            uint32_t speed = getSpeedUs();

            DRV_STATUS_t drv_status{0};
            drv_status.sr = driver->DRV_STATUS();
            Serial.println(drv_status.sg_result);

            // last speed value is dummy to prevent i+1 errors
            //uint16_t speeds[]{1365, 632, 411, 305, 242, 201, 171, 149, 133, 119, 108, 99, 91, 85, 79, 74, 69, 66, 62, 59, 0};
            //float curve[]{934.33, 730.73, 585.53, 526.87, 505.13, 513.60, 565.13, 510.80, 438.00, 424.00, 372.33, 281, 235.6, 195, 157, 108.67, 66.2, 45.4, 16.6, 1, 0};

            float curve[]{784.6,705.5,608.9,574.3,611.6,559.7,556.7,641.3,528.5,532,605.1,622.6,597.1,553.4,585.9,568.4,560,537.9,557,517.9,510.9,546.5,564.3,515.3,496,497.4,498,479.4,507.4,436.5,434.7,423.3,348.4,360.7,330.9,273.8,268.2,273.7,255.9,226.8};
            uint16_t speeds[]{3619,1809,1206,904,723,603,517,452,402,361,329,301,278,258,241,226,212,201,190,180,172,164,157,150,144,139,134,129,124,120,116,113,109,106,103,100,97,95,92,90,0};
            
            Serial.print(drv_status.sg_result);
            Serial.print(" : ");
            Serial.println(speed);

            for (int8_t i = 0; i < 40; i++)
            {
                if ((speeds[i] >= speed && speeds[i + 1] < speed) || speed >= speeds[0])
                {
                    if (curve[i] > drv_status.sg_result)
                    {
                        // Load too high, slow down
                        Serial.print("slower : ");
                        Serial.println(curve[i]);
                        setSpeedUs(min(speed + 20, speeds[0]));
                    }
                    else
                    {
                        // Load too low, speed up
                        Serial.println("faster");
                        setSpeedUs(max(speed - 20, speeds[39]));
                    }
                    break;
                }
            }
        }
    }

    // positive values turn left or move ferrari away from motor
    void rotate(float rps, uint16_t mm = 0)
    {
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

    void stop(float seconds = 0)
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

public:
    Stepper() : timer(this, &Stepper::loop)
    {
        SPI.begin();

        // Setup timer for loop (every 0,1s)
        timer.setInterval(100);
        timer.start();
    }

    void init(stepper_s config)
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
    }

    // QUEUE FUNCTIONALITY
    void add(queueItem_s item)
    {
        queue.push_back(item);
    }

    void loop(uint8_t lastX)
    {
        queueItem_s item{0, lastX, LOOP};
        add(item);
    }
    //

    // STEPPER STATUS SETTERS
    void off()
    {
        stop();
        driver->toff(0);
        queue.clear();
        totalTicks = 0;
        isHomed = false;
    }

    void on()
    {
        stop();
        driver->toff(1);
        queue.clear();
        queueCurrent = 0;
        stopped = true;
    }

    void setSpeed(float rps)
    {
        stepper->setSpeedInHz((int)microsteps_per_rotation * abs(rps));
        stepper->applySpeedAcceleration();
    }

    void position(float rps, uint16_t position)
    {
        // do not allow setting fixed end position if currently homing
        // update next command instead if it is of type POSITION
        if(stopOnStall){
            if(queue[queueCurrent].command == POSITION){
                queue[queueCurrent].mm = position;
            }
            return;
        }

        int32_t pos = position * microsteps_per_rotation / config.mm_per_rotation;
        stepper->setSpeedInHz((int)microsteps_per_rotation * abs(rps));
        stepper->moveTo(pos);
    }

    void setSpeedUs(uint32_t speed)
    {
        stepper->setSpeedInUs(speed);
        stepper->applySpeedAcceleration();
    }
    //

    // STEPPER STATUS GETTERS
    uint32_t getSpeedUs()
    {
        return abs(stepper->getCurrentSpeedInUs());
    }

    status_s getStatus()
    {
        status_s stats{0, 0, 0, 0, ""};
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
        stats.rotations = totalTicks / microsteps_per_rotation;

        return stats;
    }

    bool getHomed()
    {
        return isHomed;
    }
    //

    void loop()
    {
        if (moveFinished())
        {
            next();
            return;
        }

        moveAdjust();
    }
};