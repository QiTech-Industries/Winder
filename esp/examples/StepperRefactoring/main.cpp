#include <Stepper.h>  // Pulse generation for Stepper

stepper_s spoolConfig = {.stepperId = "spool",
                         .maxCurrent = 700,
                         .microstepsPerStep = 32,
                         .stepsPerRotation = 200,
                         .mmPerRotation = 10,
                         .pins = {
                        .en = 12,
                        .dir = 27,
                        .step = 25,
                        .cs = 2,
                         }};

Stepper spool;

void setup() {
    SPI.begin();
    Serial.begin(115200);
    spool.init(spoolConfig);
    spool.position(-40, 60);
}
void loop() {
    spool.loop();
}