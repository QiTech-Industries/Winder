#include <AccelStepper.h>
#include <TMCStepper.h>

using TMC2130_n::DRV_STATUS_t;

#define EN_PIN 12    // Enable
#define DIR_PIN 16   // Direction
#define STEP_PIN 26  // Step
#define CS_PIN 5     // Chip select
#define SW_MOSI 23   // Software Master Out Slave In (MOSI)
#define SW_MISO 19   // Software Master In Slave Out (MISO)
#define SW_SCK 18    // Software Slave Clock (SCK)

#define R_SENSE \
    0.11f  // Match to your driver                         \
                      // SilentStepStick series use 0.11              \
                      // UltiMachine Einsy and Archim2 boards use 0.2 \
                      // Panucatt BSD2660 uses 0.1                    \
                      // Watterott TMC5160 uses 0.075

// Select your stepper driver type
TMC2130Stepper driver = TMC2130Stepper(CS_PIN, R_SENSE);  // Hardware SPI

constexpr uint32_t steps_per_mm = 80;

AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);

void setup() {
    SPI.begin();
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("Start...");
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    driver.begin();           // Initiate pins and registeries
    driver.rms_current(600);  // Set stepper current to 600mA. The command is the same as command TMC2130.setCurrent(600, 0.11, 0.5);
    driver.en_pwm_mode(1);    // Enable extremely quiet stepping
    driver.pwm_autoscale(1);
    driver.microsteps(16);

    stepper.setMaxSpeed(500 * steps_per_mm);       // 100mm/s @ 80 steps/mm
    stepper.setAcceleration(1000 * steps_per_mm);  // 2000mm/s^2
    stepper.setEnablePin(EN_PIN);
    stepper.setPinsInverted(false, false, true);
    stepper.enableOutputs();
}

void loop() {
    if (stepper.distanceToGo() == 0) {
        stepper.disableOutputs();
        delay(100);
        stepper.move(100 * steps_per_mm);  // Move 100mm
        stepper.enableOutputs();
    }

    DRV_STATUS_t drv_status{0};
    drv_status.sr = driver.DRV_STATUS();

    Serial.print("0 ");
    Serial.print(drv_status.sg_result, DEC);
    Serial.print(" ");
    Serial.println(driver.cs2rms(drv_status.cs_actual), DEC);  // mA currently consumed
    stepper.run();
}
