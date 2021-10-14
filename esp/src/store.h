#ifndef INCLUDE_H
#define INCLUDE_H

#include <Arduino.h>
#include <FastAccelStepper.h>

enum connection_e
{
    OFFLINE,
    CONNECTING,
    ONLINE,
};

enum mode_e
{
    STANDBY,
    POWER,
    PULLING,
    CALIBRATING,
    WINDING,
    UNWINDING,
};

enum commands_s
{
    ROTATE,
    HOME,
    POSITION,
    LOOP,
    ADJUST
};

struct stepper_s
{
    uint16_t max_current;
    uint16_t microsteps;
    float_t gear_ratio;
    uint16_t steps_per_rotation;
    uint16_t mm_per_rotation;
    uint16_t default_acceleration;
    uint8_t stall;
    struct Pins
    {
        uint8_t en;
        uint8_t dir;
        uint8_t step;
        uint8_t cs;
    } pins;
};

struct version_s
{
    char version[50];
    char build[20];
    char date[20];
};

struct hard_config_s
{
    struct motors_s
    {
        stepper_s puller;
        stepper_s ferrari;
        stepper_s spool;
    } motors;
    struct server_s
    {
        uint16_t port;
        char default_path[32];
        char socket_path[32];
    } server;
    struct device_s
    {
        uint16_t hardware_version;
    } device;
};

struct soft_config_s
{
    struct wifi_s
    {
        char ssid[64];
        char password[34];
        char ap_ssid[64];
        char ap_password[34];
        char friendly_name[64]; // as shown in the router
        char mdns_name[64];     // {mdns_name}.local (winder domain)
        bool ap_enabled;
    } wifi;
    struct software_s
    {
        version_s spiffs;
        version_s firmware;
    } software;
    uint16_t ferrari_min;
    uint16_t ferrari_max;

public:
    String asJSON();
    void fromJSON(char* json);
    void store();
    void backup();
    bool load();
    void loadBlynkCredentials();
};

struct config_s
{
    hard_config_s hard;
    soft_config_s soft;
};

extern soft_config_s soft;
extern hard_config_s hard;
extern mode_e mode;
extern connection_e connection;
extern FastAccelStepperEngine engine;
extern String mode2string();

#endif