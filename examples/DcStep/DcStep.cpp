/**
 * Author Teemu Mäntykallio
 * Initializes the library and runs the stepper
 * motor in alternating directions.
 */

#include <TMCStepper.h>

#define EN_PIN 12           // Enable
#define DIR_PIN 16          // Direction
#define STEP_PIN 26         // Step
#define CS_PIN 5            // Chip select
#define SW_MOSI 66          // Software Master Out Slave In (MOSI)
#define SW_MISO 44          // Software Master In Slave Out (MISO)
#define SW_SCK 64           // Software Slave Clock (SCK)
#define SW_RX 63            // TMC2208/TMC2224 SoftwareSerial receive pin
#define SW_TX 40            // TMC2208/TMC2224 SoftwareSerial transmit pin
#define SERIAL_PORT Serial1 // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS 0b00 // TMC2209 Driver address according to MS1 and MS2

#define R_SENSE 0.11f // Match to your driver                         
                      // SilentStepStick series use 0.11              
                      // UltiMachine Einsy and Archim2 boards use 0.2 
                      // Panucatt BSD2660 uses 0.1                    
                      // Watterott TMC5160 uses 0.075

// Select your stepper driver type
TMC2130Stepper driver(CS_PIN, R_SENSE); // Hardware SPI
//TMC2130Stepper driver(CS_PIN, R_SENSE, SW_MOSI, SW_MISO, SW_SCK); // Software SPI
//TMC2660Stepper driver(CS_PIN, R_SENSE);                           // Hardware SPI
//TMC2660Stepper driver(CS_PIN, R_SENSE, SW_MOSI, SW_MISO, SW_SCK);
//TMC5160Stepper driver(CS_PIN, R_SENSE);
//TMC5160Stepper driver(CS_PIN, R_SENSE, SW_MOSI, SW_MISO, SW_SCK);

//TMC2208Stepper driver(&SERIAL_PORT, R_SENSE);                     // Hardware Serial
//TMC2208Stepper driver(SW_RX, SW_TX, R_SENSE);                     // Software serial
//TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);
//TMC2209Stepper driver(SW_RX, SW_TX, R_SENSE, DRIVER_ADDRESS);

void setup()
{
    pinMode(EN_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    digitalWrite(EN_PIN, LOW); // Enable driver in hardware

    // Enable one according to your setup
    SPI.begin(); // SPI drivers
    Serial.begin(115200);
    //SERIAL_PORT.begin(115200);      // HW UART drivers
    //driver.beginSerial(115200);     // SW UART drivers

    driver.begin();          //  SPI: Init CS pins and possible SW SPI pins
                             // UART: Init SW UART (if selected) with default 115200 baudrate
    driver.toff(8);          // Enables driver in software
    driver.rms_current(700); // Set motor RMS current
    driver.microsteps(16);   // Set microsteps to 1/16th

    driver.DCCTRL(100);
    driver.vhighfs(true);
    driver.vhighchm(true);
    driver.dc_time(3);
    driver.VDCMIN(0);
    driver.dc_time(450);
    driver.dc_sg(30);
    Serial.println("connection test");
    Serial.println(driver.test_connection());

    Serial.println("Trieber aktiviert");
    Serial.println(driver.dcen_cfg4());
    Serial.println(driver.dcin_cfg5());
    Serial.println("Fertig");

    //driver.en_pwm_mode(true);       // Toggle stealthChop on TMC2130/2160/5130/5160
    //driver.en_spreadCycle(false);   // Toggle spreadCycle on TMC2208/2209/2224
    //driver.pwm_autoscale(true);     // Needed for stealthChop
}

bool shaft = false;

void loop()
{
    // Run 5000 steps and switch direction in software
    for (uint16_t i = 2000; i > 0; i--)
    {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(100);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(100);
        Serial.println(driver.dcen_cfg4());
    }
    //shaft = !shaft;
    //driver.shaft(shaft);
}