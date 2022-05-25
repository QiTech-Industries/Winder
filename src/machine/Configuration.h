#pragma once

#include <ArduinoJson.h>
#include <LITTLEFS.h>
#include <Preferences.h>
#include <controller/stepper/StepperTest.h>
#include <logger/logging.h>
#include <validator/McValidator.h>

const loggingLevel_e LOG_LEVEL = INFO;
Preferences prefs;

/**
 * @brief Version of a component / configuration
 */
struct version_s {
    char version[50];  // Version, usually human-readable, examples: "milan-produktion", "v1.0.2s"
    char build[20];    // Build, usually a number, example: "01071955" meaning 7th January 19:55
    char date[20];     // Date when update was pulled: "Sat, 11 Dec 2021"
};

/**
 * @brief Hardware configuration of the winder-machine
 */
struct winderHardwareConfiguration_s {
    struct motors_s {
        stepperConfiguration_s puller;   // Configuration of the puller-motor
        stepperConfiguration_s ferrari;  // Configuration of the ferrari-motor
        stepperConfiguration_s spool;    // Configuration of the spool-motor
        float ferrariOffset;             // Offset between calibration-position (laserpointer etc.) and winding-position in mm.
                                         // Positive if winding is right of calibration, negative otherwise
    } motors;                            // Motor-related configurations
    struct server_s {
        uint16_t port;          // Port for connections, should be 80
        char default_path[32];  // Default ressource/path, for example "index.html"
        char socket_path[32];   // path for websockets, for example "/ws"
    } server;                   // Settings for the webfrontend
    struct device_s {
        uint16_t hardware_version;  // Version for identifying different hardware configurations
    } device;                       // Device related settings
};

/**
 * @brief Wifi configuration
 *
 */
struct wifiConfiguration_s {
    char ssid[64];           // SSID of an existing wifi to connect to
    char password[34];       // Password for connecting to an existing wifi
    char ap_ssid[64];        // SSID of the accesspoint to be created
    char ap_password[34];    // Login-password of the accesspoint to be created
    char friendly_name[64];  // Name the device advertises itself with e.g. in router interface
    char mdns_name[64];      // Winder dns-name in form of {mdns_name}.local (winder domain)
    bool ap_enabled;         // true = enable access point, false = only enable access point as fallback if wifi-connection fails
};

/**
 * @brief Software configuration of the winder-machine
 */
struct winderSoftwareConfiguration_s {
    wifiConfiguration_s wifi;  // Configuration details of wifi (connecting to an existing wifi
                               // and creation of an access point)
    struct software_s {
        version_s spiffs;    // Version of the webinterface
        version_s firmware;  // Version of the firmware
    } software;              // Software related versions
    uint16_t ferrari_min;    // "left" endposition for the oscillation of the ferrari
    uint16_t ferrari_max;    // "right" endposition for the oscillation of the ferrari

    /**
     * @brief Create a json-representation of the current configuration
     *
     * @return String
     */
    JsonObject toJSON();

    /**
     * @brief Parse json-formatted configuration
     *
     * @param json text in json-format to be parsed
     */
    void fromJSON(char* json);

    /**
     * @brief Save configuration to filesystem. Must be called async or esp
     * might crash on watchdog trigger
     */
    void store();

    /**
     * @brief Save JSON Config to EEPROM Backup
     */
    void backup();

    /**
     * @brief Load credentials struct from EEPROM
     */
    void loadBlynkCredentials();

    /**
     * @brief Load configuration data from filesystem
     * Also calls loadBlynkCredentials()
     *
     * @return true success, configuration loaded
     * @return false error occurred (such as filesystem not being available)
     */
    bool load();
};

/**
 * @brief Wrapper for the entire winder-machine configuration
 */
struct winderConfiguration_s {
    winderHardwareConfiguration_s hard;  // Hardware related configuration, never changed by program
    winderSoftwareConfiguration_s soft;  // Software related configuration, modified on user input

    /**
     * @brief Check if configuration parameters have unexpected value, log error
     *
     * @return true config is valid
     * @return false config is invalid
     */
    bool isValid();
};
