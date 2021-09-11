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
        String error;
    };

    std::vector<queueItem_s> queue;
    uint16_t queueCurrent;
    bool stopOnStall = false;
    bool adjustSpeedToStall = false;
    bool stopped = true;
    int ignore = 25;
    uint16_t windAccel = 1000;

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
            stepper->forceStopAndNewPosition(stepper->getCurrentPosition()); //Stop RAMPS generator
            stopOnStall = false;                                             // StallGuard deactivated
            driver->toff(1);                                                 // Activate Stepper driver
            adjustSpeedToStall = false;                                      //
            stopped = false;
            stepper->setAcceleration(1000);

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
                rotate(item.rps);
                break;
            case POSITION:
                position(item.rps, item.mm);
                break;
            case ADJUST:
                adjustSpeedToStall = true;
                stepper->setAcceleration(300);
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
        if (!stepper->isMotorRunning())
        {
            //DEBUG_PRINTF("STATS: %i %i %i \n", stepper->isMotorRunning(), stepper->isRampGeneratorActive(), stepper->isQueueEmpty());
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
                return true;
            }
        }

        return false;
    }

    void moveAdjust()
    {
        if (adjustSpeedToStall)
        {
           /* if (windAccel >= 200)
            {
                windAccel = windAccel * 0.99;
                stepper->setAcceleration(windAccel);
                stepper->applySpeedAcceleration();
            }*/

            uint32_t speed = getSpeedUs();

            DRV_STATUS_t drv_status{0};
            drv_status.sr = driver->DRV_STATUS();

            // defines range where stall value should be in
            float calcMiddle = 0.19424 * speed + 340;
            float calcLow = 0.19424 * speed + 239;
            float calcHigh = 0.19424 * speed + 440;

            DEBUG_PRINTF("%u %f %f %f\n", drv_status.sg_result, calcLow, calcMiddle, calcHigh);

            if (drv_status.sg_result < calcLow)
            {
                setSpeed(0.015); //0.5m/s with filled up spool
            }
            else if (drv_status.sg_result > calcHigh)
            {
                setSpeed(0.78); // 0.5s per rotation
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
        this->microsteps_per_rotation = config.steps_per_rotation * config.microsteps / config.gear_ratio;

        // DRIVER config
        driver->toff(0);
        driver->blank_time(5);
        driver->rms_current(config.max_current);
        driver->microsteps(config.microsteps);
        driver->sgt(7);
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
            stepper->setAcceleration(1000);
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
    }

    void on()
    {
        driver->toff(1);
    }

    void setSpeed(float rps)
    {
        DEBUG_PRINTF("SPEED %f", rps);
        stepper->setSpeedInHz((int)microsteps_per_rotation * abs(rps));
        stepper->applySpeedAcceleration();
    }

    void position(float rps, uint16_t position)
    {
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
        status_s stats{0,0,0,""};
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
            DEBUG_PRINTLN(stats.rpm);
        }

        stats.stall = drv_status.sg_result;
        stats.active = driver->toff() == 0 ? 0 : 1;

        return stats;
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