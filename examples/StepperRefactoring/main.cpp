#include <Stepper.h>  // Pulse generation for Stepper

stepper_s spoolConfig = {.stepperId = "spool",
                         .maxCurrent = 700,
                         .microstepsPerStep = 32,
                         .stepsPerRotation = 200,
                         .mmPerRotation = 10,
                         /*
                         .pins = { // Pins of puller
                        .en = 12,
                        .dir = 27,
                        .step = 25,
                        .cs = 2,
                         }
                         */
                        .pins = { // Pins of ferrari
                        .en = 12,
                        .dir = 14,
                        .step = 17,
                        .cs = 13,
                         }
};

Stepper spool;

void setup() {
    SPI.begin();
    Serial.begin(115200);

    spool.init(spoolConfig);
    spool.setDebugging(false);
    spool.movePosition(40, 100);
}

void loop() {
    // Handle commands sent by user via terminal
    if(Serial.available() > 0){
        uint8_t newCommand = Serial.read();
        switch(newCommand){
            // Basic commands
            case 'h': // Home
                Serial.println("[CMD]: home()");
                spool.moveHome(30);
                break;
            case 'o': // Oscillate
                Serial.println("[CMD]: moveOscillate()");
                spool.moveOscillate(40, 50, 90);
                break;
            case 'p': // Position
                Serial.println("[CMD]: movePosition()");
                spool.movePosition(40, 80);
                break;
            case 'r': // Rotate
                Serial.println("[CMD]: moveRotate()");
                spool.moveRotate(40);
                break;
            case 'a': // Adjust
                Serial.println("[CMD]: moveRotateWithLoadAdjust()");
                spool.moveRotateWithLoadAdjust(40, 60);
                break;
            case 'x': // mode off
                Serial.println("[CMD]: switchModeOff()");
                spool.switchModeOff();
                break;
            case 's': // mode standby
                Serial.println("[CMD]: switchModeStandby()");
                spool.switchModeStandby();
                break;
            case 'l': // print out long debugging message
                Serial.println("[CMD]: enable debugging, printStatusLong()");
                spool.printStatus(true);
                break;
            case 'd': // enable debugging
                Serial.println("[CMD]: enable debugging");
                spool.setDebugging(true);
                break;
            case 'D': // disable  debugging
                Serial.println("[CMD]: disable debugging");
                spool.setDebugging(false);
                break;
            // Debug-related 'programs', adjust as needed
            case '1':
                Serial.println("[CMD]: debug procedure 1");
                break;
            case '2':
                Serial.println("[CMD]: debug procedure 2");
                break;
            case '3':
                Serial.println("[CMD]: debug procedure 3");
                break;
            case '4':
                Serial.println("[CMD]: debug procedure 4");
                break;
            case '5':
                Serial.println("[CMD]: debug procedure 5");
                break;
            case '6':
                Serial.println("[CMD]: debug procedure 6");
                break;
            default:
                Serial.println("[CMD] Unknown command - input ignored");
        };
    }

    // Let the stepper do its thing for a second
    for(int i=0; i<10; ++i){
        for(int k=0; k<10; ++k){
            spool.handle();
            delay(10); // TODO: Delay only to prevent a debug-message-flood
        }
        spool.printStatus(false);
    }
}