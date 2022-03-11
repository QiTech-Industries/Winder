#pragma once

// Related
// System / External
#include <Arduino.h>
#include <FastAccelStepper.h>
// Selfmade
#include "controller/stepper/Stepper.h"
// Project

// TODO: Include magic inside configuration to avoid broken configurations

/**
 * @brief Version of a component / configuration
 * TODO: numbers (minor/major...) instead of char-arrays?
 */
struct version_s {
    char version[50]; // Version, usually human-readable, examples: "milan-produktion", "v1.0.2s"
    char build[20]; // Build, usually a number, example: "01071955"
    char date[20]; // Date of the build, example: "Sat, 11 Dec 2021"
};

/**
 * @brief Hardware configuration of the winder-machine
 */
struct configurationMachineWinderHardware_s {
    struct motors_s {
        stepperConfiguration_s puller; // Configuration of the puller-motor
        stepperConfiguration_s ferrari; // Configuration of the ferrari-motor
        stepperConfiguration_s spool; // Configuration of the spool-motor
    } motors; // Motor-related configurations
    struct server_s {
        uint16_t port; // TODO: Comment
        char default_path[32]; // TODO: Comment
        char socket_path[32]; // TODO: Comment
    } server; // TODO: Comment
    struct device_s {
        uint16_t hardware_version; // TODO: Comment
    } device; // TODO: Comment
};

struct configurationWifi_s {
    char ssid[64]; // TODO: Comment
    char password[34]; // TODO: Comment
    char ap_ssid[64]; // TODO: Comment
    char ap_password[34]; // TODO: Comment
    char friendly_name[64]; // TODO: Comment // as shown in the router
    char mdns_name[64]; // TODO: Comment     // {mdns_name}.local (winder domain)
    bool ap_enabled; // TODO: Comment
};

/**
 * @brief Software configuration of the winder-machine
 */
struct configurationMachineWinderSoftware_s {
    configurationWifi_s wifi; // TODO: Comment
    struct software_s {
        version_s spiffs; // TODO: Comment
        version_s firmware; // TODO: Comment
    } software; // TODO: Comment
    uint16_t ferrari_min; // TODO: Comment
    uint16_t ferrari_max; // TODO: Comment

public:
    /**
     * @brief Create a json-representation of the current configuration
     * 
     * @return String 
     */
    String asJSON();

    /**
     * @brief Parse json-formatted configuration
     * 
     * @param json text in json-format to be parsed
     */
    void fromJSON(char* json);

    /**
     * @brief TODO: Save configuration to filesystem. Must be called async or esp might crash on watchdog trigger
     */
    void store();

    /**
     * @brief TODO: Save JSON Config to EEPROM Backup
     */
    void backup();

    /**
     * @brief Load credentials struct from EEPROM
     */
    void loadBlynkCredentials();

    /**
     * @brief TODO: Load configuration data from filesystem
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
struct configurationMachineWinder_s {
    configurationMachineWinderHardware_s hard; // Hardware related configuration
    configurationMachineWinderSoftware_s soft; // Software related configuration
};
