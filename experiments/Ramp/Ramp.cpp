#include <FastAccelStepper.h>
#include <TMCStepper.h>

#define EN_PIN 12    // Enable
#define DIR_PIN 16   // Direction
#define STEP_PIN 26  // Step
#define CS_PIN 5     // Chip select

TMC2130Stepper driver(CS_PIN, 0.11f);  // Hardware SPI
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

void setup() {
    SPI.begin();  // SPI drivers
    Serial.begin(115200);

    driver.begin();           //  SPI: Init CS pins and possible SW SPI pins
    driver.toff(8);           // Enables driver in software
    driver.rms_current(700);  // Set motor RMS current
    driver.microsteps(16);    // Set microsteps to 1/16th

    engine.init();
    stepper = engine.stepperConnectToPin(STEP_PIN);
    stepper->setDirectionPin(DIR_PIN);
    stepper->setEnablePin(EN_PIN);
    stepper->setAutoEnable(true);
    stepper->setSpeedInUs(10000);  // the parameter is us/step !!!
    stepper->setAcceleration(10000);
    stepper->moveTo(60000);
}

void loop() {
    // Serial.println(stepper->getCurrentPosition());
    // stepper->setSpeedInUs(100);
    // if(stepper->getCurrentPosition() > 19990){
    //     stepper->moveTo(40000);
    //     stepper->moveTo(8000);
    //     delay(500);
    //     stepper->moveTo(10000);
    // }
    if (!stepper->isMotorRunning()) {
        Serial.println("FINISHED");
    }
}
