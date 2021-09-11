#include <TMCStepper.h>
#include "FastAccelStepper.h"

using TMC2130_n::DRV_STATUS_t;

// As in StepperDemo for Motor 1 on AVR
//#define dirPinStepper    5
//#define enablePinStepper 6
//#define stepPinStepper   9  // OC1A in case of AVR

// As in StepperDemo for Motor 1 on ESP32
#define dirPinStepper 16
#define enablePinStepper 12
#define stepPinStepper 26
#define CS_PIN 5 // Chip select

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;
TMC2130Stepper driver(CS_PIN, 0.11f);

void setup()
{
    Serial.begin(115200);

    SPI.begin();

    // https://www.trinamic.com/fileadmin/assets/Support/AppNotes/AN002-stallGuard2.pdf
    driver.begin();
    driver.toff(20);
    driver.blank_time(54);
    driver.rms_current(1200);
    driver.microsteps(64);
    driver.en_pwm_mode(1); // Enable extremely quiet stepping

    driver.TCOOLTHRS(0xFFFFF);
    driver.seimin(true);
    driver.semin(5);
    driver.sgt(3);

    engine.init();
    stepper = engine.stepperConnectToPin(stepPinStepper);
    if (stepper)
    {
        stepper->setDirectionPin(dirPinStepper);
        stepper->setEnablePin(enablePinStepper);
        stepper->setAutoEnable(true);

        // If auto enable/disable need delays, just add (one or both):
        // stepper->setDelayToEnable(50);
        // stepper->setDelayToDisable(1000);

        stepper->setSpeedInUs(100); // the parameter is us/step !!!
        stepper->setAcceleration(1000);
        stepper->move(10000);
    }
}

int callCount = 0;

void loop()
{
    static uint32_t last_time = 0;
    uint32_t ms = millis();

    if ((ms - last_time) > 100)
    { //run every 0.1s
        callCount++;
        last_time = ms;

        DRV_STATUS_t drv_status{0};
        drv_status.sr = driver.DRV_STATUS();

        Serial.println(drv_status.otpw); //Overtemperatur warning (120°C pre-warning and 150°C thermal shutdown
        Serial.print(drv_status.sg_result);  //Stall Guard result
        Serial.print(" ");
        Serial.println(driver.cs2rms(drv_status.cs_actual), DEC); // mA currently consumed
    }

    if (callCount == 100)
    {
        stepper->runBackward();
    }
}