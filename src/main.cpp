// Related
// System / External
// Selfmade
// Project
#include <QiMachineWinder.h>

QiMachineWinder JarvisWinder;

void setup()
{
    SPI.begin();
    Serial.begin(115200);
    configurationMachineWinder_s conf = {
        .hard = {
            .motors = {
                .puller = {
                    .stepperId = "puller",
                    .maxCurrent = 700,
                    .microstepsPerStep = 32,
                    .stepsPerRotation = 200,
                    .mmPerRotation = 108,
                    .gearRatio = 1,
                    //.default_acceleration = 10000,
                    //.stall = 7,
                    .pins = {
                        .en = 12,
                        .dir = 27,
                        .step = 25,
                        .cs = 2,
                    }
                },
                .ferrari = {
                    .stepperId = "ferrari",
                    .maxCurrent = 700,
                    .microstepsPerStep = 32,
                    .stepsPerRotation = 200,
                    .mmPerRotation = 8,
                    .gearRatio = 1,
                    //.default_acceleration = 10000,
                    //.stall = 7,
                    .pins = {
                        .en = 12,
                        .dir = 14,
                        .step = 17,
                        .cs = 13,
                    }
                },
                .spool = {
                    .stepperId = "spool",
                    .maxCurrent = 700,
                    .microstepsPerStep = 32,
                    .stepsPerRotation = 200,
                    .mmPerRotation = 2800,
                    .gearRatio = 5.18,
                    //.default_acceleration = 10000,
                    //.stall = 8, // 310 if spool is not filled up
                    .pins = {
                        .en = 12,
                        .dir = 16,
                        .step = 26,
                        .cs = 5,
                    }
                },
                .ferrariOffset = 0 // 0 for old ferrari, 11-ish for new ferrari
            },
            .server = {
                .port = 80,
                "index.html",
                "/ws"
            },
            .device = {
                .hardware_version = 1,
            }},
        .soft = {
            .wifi = {
                // TODO: fix struct-initialisation
                "QiTech", // .ssid = 
                "Malte100%", // .password = 
                "Jarvis Winder", // .ap_ssid = 
                "", // .ap_password = 
                "Jarvis Winder", // .friendly_name = 
                "winder", // .mdns_name = 
                true, // .ap_enabled = 
            },
            .software = {
                .spiffs = {"0.0.1", "1209211146"},
                .firmware = {"0.0.1", "1209211146"},
            },
            .ferrari_min = 80,
            .ferrari_max = 140,
        }};

    JarvisWinder.init(conf);
}

void loop() {
    // Interactive key-input based debugging
    /*
    if(Serial.available() > 0){
        uint8_t newCommand = Serial.read();
        JarvisWinder.executeDebugCommand(newCommand);
    }
    */

    JarvisWinder.handle();
}
