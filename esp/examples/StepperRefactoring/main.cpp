#include <Stepper.h>  // Pulse generation for Stepper

stepper_s spoolConfig = {.stepperId = "spool",
                         .maxCurrent = 700,
                         .microstepsPerStep = 32,
                         .stepsPerRotation = 200,
                         .mmPerRotation = 10,
                         .pins = {
                             .en = 12,
                             .dir = 16,
                             .step = 26,
                             .cs = 5,
                         }};

Stepper spool(spoolConfig);

void setup() {
    Serial.begin(115200);
    spool.rotate(5);
}
void loop() { spool.loop(); }