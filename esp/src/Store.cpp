#include <store.h>
#include <ArduinoJSON.h>
#include <LITTLEFS.h>

void cpy(char *dst, const char *src)
{
    strlcpy(dst, src, sizeof(dst));
}

String SoftConfig::asJSON()
{
    StaticJsonDocument<256> doc;
    String json;

    doc["wifi"]["ssid"] = wifi.ssid;
    doc["wifi"]["password"] = wifi.password;
    doc["wifi"]["ap_ssid"] = wifi.ap_ssid;
    doc["wifi"]["ap_password"] = wifi.ap_password;
    doc["wifi"]["friendly_name"] = wifi.friendly_name;
    doc["wifi"]["mdns_name"] = wifi.mdns_name;
    doc["wifi"]["ap_enabled"] = wifi.ap_enabled;
    doc["ferrari_min"] = ferrari_min;
    doc["ferrari_max"] = ferrari_max;

    if (serializeJson(doc, json) == 0)
    {
        Serial.println("JSON serialization failed!");
    }

    return json;
}

void SoftConfig::fromJSON(String json)
{
    StaticJsonDocument<256> doc;

    if (deserializeJson(doc, json) == 0)
    {
        Serial.println("JSON deserialization failed!");
    }

    cpy(wifi.ssid, doc["wifi"]["ssid"]);
    cpy(wifi.password, doc["wifi"]["password"]);
    cpy(wifi.ap_ssid, doc["wifi"]["ap_ssid"]);
    cpy(wifi.ap_password, doc["wifi"]["ap_password"]);
    cpy(wifi.friendly_name, doc["wifi"]["friendly_name"]);
    cpy(wifi.mdns_name, doc["wifi"]["mdns_name"]);
    wifi.ap_enabled = doc["wifi"]["ap_enabled"];
    ferrari_min = doc["wifi"]["ferrari_min"];
    ferrari_max = doc["wifi"]["ferrari_max"];
}

void SoftConfig::set(SoftConfig conf)
{
    File f = LITTLEFS.open("/winder.conf", "w");
    if (f)
    {
        f.write((byte *)&conf, sizeof(conf));
        Serial.println("writing config");
    }
    f.close();
}

String mode2string()
{
    switch (mode)
    {
    case POWER:
        return "power";
        break;
    case HOMING:
        return "homing";
        break;
    case CALIBRATING:
        return "calibrating";
        break;
    case WINDING:
        return "winding";
        break;
    case UNWINDING:
        return "unwinding";
        break;
    default:
        return "standby";
        break;
    }
}

SoftConfig soft;
HardConfig hard;
mode_e mode = STANDBY;
FastAccelStepperEngine engine;
connection_e connection = OFFLINE;