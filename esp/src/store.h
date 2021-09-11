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
    HOMING,
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

struct BlynkStore
{
    uint32_t magic;
    char version[15];
    uint8_t flags;
    char wifiSSID[34];
    char wifiPass[64];
    char cloudToken[34];
    char cloudHost[34];
    uint16_t cloudPort;
    uint32_t staticIP;
    uint32_t staticMask;
    uint32_t staticGW;
    uint32_t staticDNS;
    uint32_t staticDNS2;
    int last_error;
};

struct stepper_s
{
    uint16_t max_current;
    uint16_t microsteps;
    float_t gear_ratio;
    uint16_t steps_per_rotation;
    uint16_t mm_per_rotation;
    struct Pins
    {
        uint8_t en;
        uint8_t dir;
        uint8_t step;
        uint8_t cs;
    } pins;
};

struct Version
{
    char build[8];
    char version[8];
};

struct HardConfig
{
    struct Motors
    {
        stepper_s puller;
        stepper_s ferrari;
        stepper_s spool;
    } motors;
    struct Server
    {
        uint16_t port;
        char default_path[32];
        char socket_path[32];
    } server;
    struct Software
    {
        Version spiffs;
        Version firmware;
    } software;
};

struct SoftConfig
{
    struct Wifi
    {
        char ssid[64];
        char password[34];
        char ap_ssid[64];
        char ap_password[34];
        char friendly_name[64]; // as shown in the router
        char mdns_name[64];     // {mdns_name}.local (winder domain)
        bool ap_enabled;
    } wifi;
    uint16_t ferrari_min;
    uint16_t ferrari_max;

public:
    String asJSON();
    void fromJSON(String json);
    void set(SoftConfig conf);
};

struct Config
{
    HardConfig hard;
    SoftConfig soft;
};

extern SoftConfig soft;
extern HardConfig hard;
extern mode_e mode;
extern connection_e connection;
extern FastAccelStepperEngine engine;
extern String mode2string();

#endif