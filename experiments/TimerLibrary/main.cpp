#include "timer.h"

// Create the timer instance
Timer timer;

void helloCallback() { Serial.println("Hello"); }

void setup() {
    Serial.begin(115200);

    // The timer will repeat every 1000 ms
    timer.setInterval(1000);

    // The function to be called
    timer.setCallback(helloCallback);

    // Start the timer
    timer.start();
}

void loop() {
    Serial.println("Loop");
    // Update the timer
    timer.update();
}
