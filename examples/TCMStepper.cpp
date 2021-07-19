#include <TMCStepper.h>

#define EN_PIN 12           // Enable
#define DIR_PIN 16          // Direction
#define STEP_PIN 26         // Step
#define CS_PIN 5            // Chip select
#define SW_MOSI 23          // Software Master Out Slave In (MOSI)
#define SW_MISO 19          // Software Master In Slave Out (MISO)
#define SW_SCK 18           // Software Slave Clock (SCK)
#define SW_RX 63            // TMC2208/TMC2224 SoftwareSerial receive pin
#define SW_TX 40            // TMC2208/TMC2224 SoftwareSerial transmit pin
#define SERIAL_PORT Serial1 // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS 0b00 // TMC2209 Driver address according to MS1 and MS2

#define R_SENSE 0.11f // SilentStepStick series use 0.11

// stepper driver type
TMC2130Stepper driver(CS_PIN, R_SENSE); // Hardware SPI

void setup()
{
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW); // Enable driver in hardware

  SPI.begin(); // SPI drivers

  driver.begin();             //  SPI: Init CS pins and possible SW SPI pins
  driver.toff(5);             // Enables driver in software
  driver.rms_current(600);    // Set motor RMS current
  driver.microsteps(16);      // Set microsteps to 1/16th
  driver.pwm_autoscale(true); // Needed for stealthChop
}

bool shaft = false;

void loop()
{
  // Run 5000 steps and switch direction in software
  for (uint16_t i = 5000; i > 0; i--)
  {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(160);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(160);
  }
  shaft = !shaft;
  driver.shaft(shaft);
}