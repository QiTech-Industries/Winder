#ifndef INCLUDE_H
#define INCLUDE_H

#include <Arduino.h>
#include <FastAccelStepper.h>

// TODO: rename file, as "store.h" is misleading

/**
 * @brief TODO: Comment
 */
enum connection_e
{
    OFFLINE, // TODO: Comment
    CONNECTING, // TODO: Comment
    ONLINE, // TODO: Comment
};

/**
 * @brief TODO: Comment
 */
enum mode_e
{
    STANDBY, // TODO: Comment
    POWER, // TODO: Comment
    PULLING, // TODO: Comment
    CALIBRATING, // TODO: Comment
    WINDING, // TODO: Comment
    UNWINDING, // TODO: Comment
    CHANGING // TODO: Comment
};

/**
 * @brief TODO: Comment
 */
enum commands_s
{
    ROTATE, // TODO: Comment
    HOME, // TODO: Comment
    POSITION, // TODO: Comment
    LOOP, // TODO: Comment
    ADJUST // TODO: Comment
};

/**
 * @brief TODO: Comment
 */
struct stepper_s
{
    uint16_t max_current; // TODO: Comment
    uint16_t microsteps; // TODO: Comment
    float_t gear_ratio; // TODO: Comment
    uint16_t steps_per_rotation; // TODO: Comment
    uint16_t mm_per_rotation; // TODO: Comment
    uint16_t default_acceleration; // TODO: Comment
    uint8_t stall; // TODO: Comment
    struct Pins
    {
        uint8_t en; // TODO: Comment
        uint8_t dir; // TODO: Comment
        uint8_t step; // TODO: Comment
        uint8_t cs; // TODO: Comment
    } pins; // TODO: Comment
};

/**
 * @brief TODO: Comment
 * TODO: const numbers (minor/major...) instead of char-arrays?
 */
struct version_s
{
    char version[50]; // TODO: Comment
    char build[20]; // TODO: Comment
    char date[20]; // TODO: Comment
};

/**
 * @brief TODO: Comment
 */
struct hard_config_s
{
    struct motors_s
    {
        stepper_s puller; // TODO: Comment
        stepper_s ferrari; // TODO: Comment
        stepper_s spool; // TODO: Comment
    } motors; // TODO: Comment
    struct server_s
    {
        uint16_t port; // TODO: Comment
        char default_path[32]; // TODO: Comment
        char socket_path[32]; // TODO: Comment
    } server; // TODO: Comment
    struct device_s
    {
        uint16_t hardware_version; // TODO: Comment
    } device; // TODO: Comment
};

/**
 * @brief TODO: Comment
 */
struct soft_config_s
{
    struct wifi_s
    {
        char ssid[64]; // TODO: Comment
        char password[34]; // TODO: Comment
        char ap_ssid[64]; // TODO: Comment
        char ap_password[34]; // TODO: Comment
        char friendly_name[64]; // TODO: Comment // as shown in the router
        char mdns_name[64]; // TODO: Comment     // {mdns_name}.local (winder domain)
        bool ap_enabled; // TODO: Comment
    } wifi; // TODO: Comment
    struct software_s
    {
        version_s spiffs; // TODO: Comment
        version_s firmware; // TODO: Comment
    } software; // TODO: Comment
    uint16_t ferrari_min; // TODO: Comment
    uint16_t ferrari_max; // TODO: Comment

/**
 * @brief TODO: Comment
 * TODO: Public methods... on a struct?!?
 */
public:
    String asJSON(); // TODO: Comment
    void fromJSON(char* json); // TODO: Comment
    void store(); // TODO: Comment
    void backup(); // TODO: Comment
    bool load(); // TODO: Comment
    void loadBlynkCredentials(); // TODO: Comment
};

/**
 * @brief TODO: Comment
 */
struct config_s
{
    hard_config_s hard; // TODO: Comment
    soft_config_s soft; // TODO: Comment
};

// TODO: Do we really need to create these global vars?
extern soft_config_s soft; // TODO: Comment
extern hard_config_s hard; // TODO: Comment
extern mode_e mode; // TODO: Comment
extern connection_e connection; // TODO: Comment
extern FastAccelStepperEngine engine; // TODO: Comment
extern String mode2string(); // TODO: Comment

#endif