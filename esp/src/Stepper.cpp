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

    void calibrateStall(uint16_t minSpeedUs)
    {
        setSpeedUs(minSpeedUs);
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
            float curve[]{660, 600, 580, 500.4, 465.6, 467.5, 401, 463.1, 371.3, 444.4, 374.2, 380.5, 364.7, 332.1, 387.9, 324.8, 323, 350, 356.4, 356.9, 309.5, 331, 324, 343.3, 316.4, 346.7, 311, 322, 300.2, 284.1, 326.3, 315.5, 320.5, 295.1, 282.7, 288, 315.1, 268.3, 265.5, 274.7};
            uint16_t speeds[]{3619, 1809, 1206, 904, 723, 603, 517, 452, 402, 361, 329, 301, 278, 258, 241, 226, 212, 201, 190, 180, 172, 164, 157, 150, 144, 139, 134, 129, 124, 120, 116, 113, 109, 106, 103, 100, 97, 95, 92, 90, 0};

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
                        setSpeedUs(min(speed * 1.1, speeds[0]));
                    }
                    else
                    {
                        // Load too low, speed up
                        Serial.println("faster");
                        setSpeedUs(max(speed * 0.8, speeds[39]));
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
        isHomed = false;
        ignore = 25;
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

    float_t getSpeedRpm()
    {
        return (60000000 / abs(stepper->getCurrentSpeedInUs()) / microsteps_per_rotation);
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
        stats.rotations = 0;

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