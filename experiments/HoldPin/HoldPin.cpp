/*
This example demonstrates how the Analog Pin 4
(HoldPin on the CNC Shield) can be used with a pushbutton.
Therefore the Button needs to be connected to 3,3V and Hold.
*/

#include <Arduino.h>

void setup() { Serial.begin(115200); }

void loop() {
    delay(1000);
    if (analogRead(4) > 4050) {
        Serial.print("Button pushed");
    }
}
