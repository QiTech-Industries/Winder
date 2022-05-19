#include <TMCStepper.h>

uint8_t pin_en = 17;     // Enable
uint8_t pin_dir = 19;    // Direction
uint8_t pin_step = 33;   // Step
uint8_t pin_cs = 34;     // Chip select
float_t rsense = 0.11f;  // SilentStepStick series use 0.11

// stepper driver type
TMC2130Stepper driver(pin_cs, rsense);  // Hardware SPI

void setup() {
    pinMode(pin_en, OUTPUT);
    pinMode(pin_dir, OUTPUT);
    pinMode(pin_step, OUTPUT);
    digitalWrite(pin_en, LOW);  // Enable driver in hardware
    pinMode(0, OUTPUT);
    digitalWrite(0, HIGH);  // Enable driver in hardware

    SPI.begin(14, 12, 13);  // Enable communication to drivers via SPI protocol

    driver.begin();              // SPI: Init CS pins and possible SW SPI pins
    driver.toff(5);              // Enables driver in software
    driver.rms_current(600);     // Set motor RMS current
    driver.microsteps(16);       // Set microsteps to 1/16th
    driver.pwm_autoscale(true);  // Needed for stealthChop
    driver.en_pwm_mode(1);       // Enable extremely quiet stepping
}

bool shaft = false;

void loop() {
    // Run 5000 steps and switch direction in software
    for (uint16_t i = 5000; i > 0; i--) {
        digitalWrite(pin_step, HIGH);
        delayMicroseconds(160);
        digitalWrite(pin_step, LOW);
        delayMicroseconds(160);
    }
    shaft = !shaft;
    driver.shaft(shaft);  // Set direction of rotation
}
