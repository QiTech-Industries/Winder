#include <machine/Setup.h>
#include <networking/QiWifi.h>

Winder JarvisWinder;

void setup() {
    SPI.begin();
    Serial.begin(115200);
    winderConfiguration_s conf = {
        .hard = {
            .motors = {
                .puller = {
                    .stepperId = "puller",
                    .maxCurrent = 700,
                    .microstepsPerStep = 32,
                    .stepsPerRotation = 200,
                    .mmPerRotation = 108,
                    .gearRatio = 1,
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
                .default_path = "index.html",
                .socket_path = "/ws"
            },
            .device = {
                .hardware_version = 1,
            }},
        .soft = {
            .wifi = {
                .ssid = "MyNetworkSsid",
                .password = "MyNetworkPassword",
                .ap_ssid = "Jarvis Winder",
                .ap_password = "", 
                .friendly_name = "Jarvis Winder",
                .mdns_name = "winder",
                .ap_enabled = true,
            },
            .software = {
                .spiffs = {
                    .version = "not-ready-for-shipping",
                    .build = "needs-remote-update",
                    .date = ""
                },
                .firmware = {
                    .version = "not-ready-for-shipping",
                    .build = "needs-remote-update",
                    .date = ""                  
                },
            },
            .ferrari_min = 80,
            .ferrari_max = 140,
        }};

    JarvisWinder.init(conf);
}

void loop() { JarvisWinder.handle(); }
