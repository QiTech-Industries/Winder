#include <TMCStepper.h>

using TMC2130_n::DRV_STATUS_t;

#define EN_PIN 12    // Enable
#define DIR_PIN 16   // Direction
#define STEP_PIN 26  // Step
#define CS_PIN 5     // Chip select

#define R_SENSE 0.11f  // Internal sense rgisters, match to your driver

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;  // necessary for locking and unlocking ports
TMC2130Stepper driver(CS_PIN, R_SENSE);                // Hardware SPI

// Using direct register manipulation can reach faster stepping times
#define STEP_PORT PORTF  // Match with STEP_PIN
#define STEP_BIT_POS 0   // Match with STEP_PIN

void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);  // lock count variable to prevent override from main loop
    digitalWrite(STEP_PIN, !digitalRead(STEP_PIN));
    portEXIT_CRITICAL_ISR(&timerMux);  // Comment out enter / exit to deactivate the critical section
}

void setup() {
    // Set timer interrupt that generates pulses
    timer = timerBegin(0, 80, true);  // set timer to microseconds divider (80 MHz / 80 = 1 MHz, as for 1 microsconds)
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 13, true);
    timerAlarmEnable(timer);  // enable alarm

    SPI.begin();
    Serial.begin(115200);  // Init serial port and set baudrate
    while (!Serial)
        ;  // Wait for serial port to connect
    Serial.println("\nStart...");

    pinMode(EN_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(CS_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(MISO, INPUT_PULLUP);
    digitalWrite(EN_PIN, LOW);

    // https://www.trinamic.com/fileadmin/assets/Support/AppNotes/AN002-stallGuard2.pdf
    driver.begin();
    driver.toff(2);           // 0-15 default: 2, duration of slow decay phase
    driver.blank_time(54);    // 16, 24, 36, 54 default: 36
    driver.rms_current(600);  // RootMeanSquare (Quadratisches Mittel der Stromstärke)
    driver.microsteps(32);    // One software Step becomes 1/16th of a hardware Step
                              // no increase in accuracy (on load) only smoother running, loss of torque
    driver.shaft(true);       // Motor direction: true left, false right

    // Cool Step config
    driver.TCOOLTHRS(0xFFFFF);  // 0..1,048,575  lower threshold velocity for switching on smart energy coolStep and stallGuard feature
    driver.THIGH(0);  // 0..1,048,575 allows velocity dependent switching into a different chopper mode and fullstepping to maximize torque.
    driver.seimin(true);  // start with 1/4 of current
    driver.semin(5);      // Current increase triggered if StallGuard < (SEMIN * 32)
                          // 0 disables Coolstep needed for configuration, Range: SG_MAX / 4 … SG_MAX / 8
                          // minimum stall value that triggers current increase
    driver.semax(0);      // current decrease triggered if  StallGuard > (SEMIN + SEMAX + 1) * 32.
    driver.sedn(10);      // Coolstep Current Decreasing Speed, 0 = slowest decrease
    driver.seup(1);       // Coolstep Current Increasing Speed, 0 = slowest increase

    // Stall Guard config
    driver.sgt(0);  // [-64..63] higher value makes stallGuard less sensitive and requires more torque to indicate a stall
}

void loop() {
    static uint32_t last_time = 0;
    uint32_t ms = millis();

    if ((ms - last_time) > 100) {  // run every 0.1s
        last_time = ms;

        DRV_STATUS_t drv_status{0};
        drv_status.sr = driver.DRV_STATUS();

        Serial.println(driver.stallguard());  // Stall Guard True or False
        Serial.print(drv_status.sg_result);   // Stall Guard result
        Serial.print(" ");
        Serial.println(driver.cs2rms(drv_status.cs_actual), DEC);  // mA currently consumed
    }
}
