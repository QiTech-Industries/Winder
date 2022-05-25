// Adapted from https://diyprojects.io/esp32-timers-alarms-interrupts-arduino-code#.YPbv3egzZmM

#include <Arduino.h>

volatile int count;   // volatile variables are only in RAM and instantly accesible from main loop
int totalInterrupts;  // counts the number of triggering of the alarm

hw_timer_t* timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;  // necessary for locking and unlocking ports

// Code with critical section
// Do not do heavy Task like Serial.print in here, justt variable changes
void IRAM_ATTR onTime()  // timers should only call routines insie RAM
{
    portENTER_CRITICAL_ISR(&timerMux);  // lock count variable to prevent override from main loop
    count++;
    portEXIT_CRITICAL_ISR(&timerMux);  // Comment out enter / exit to deactivate the critical section
}

void setup() {
    Serial.begin(115200);

    timer = timerBegin(0, 80, true);  // id  the Timer number from 0 to 3
                                      // prescale  the value of the time divider (80Mhz/80 = 1000000/s)
                                      // flag true to count on the rising edge, false to count on the falling edge

    timerAttachInterrupt(timer, &onTime, true);  // timer is the pointer to the Timer we have just created
                                                 // function the function that will be executed each time the Timer alarm is triggered
                                                 // Trigger indicates how to synchronize the Timer trigger with the clock.

    timerAlarmWrite(timer, 1000000,
                    true);  // timer the pointer to the Timer created previously
                            //  frequency the frequency of triggering of the alarm in ticks. For an ESP32 , there are 1,000,000 tics per
                            //  second autoreload true to reset the alarm automatically after each trigger.

    timerAlarmEnable(timer);  // activate timer
}

void loop() {
    if (count > 0)  // There are interrupts to handle
    {
        portENTER_CRITICAL(&timerMux);  // lock count variable to prevent override from interrupt
        count--;
        portEXIT_CRITICAL(&timerMux);

        totalInterrupts++;
        Serial.print("totalInterrupts");
        Serial.println(totalInterrupts);
    }
}
