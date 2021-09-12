#include <store.h>
#include <ArduinoJSON.h>
#include <LITTLEFS.h>
#include <Preferences.h>

// Copy String or char into struct
///////////////////////////////////
void cpy(char *dst, const char *src)
{
    strlcpy(dst, src, sizeof(dst));
}

void cpy(String dst, const char *src)
{
    strlcpy((char *)dst.c_str(), src, sizeof(dst));
}
///////////////////////////////////

String mode2string()
{
    // Convert Mode enum to String
    ///////////////////////////////////
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
    ///////////////////////////////////
}

Preferences prefs;

String soft_config_s::asJSON()
{
    // Convert struct to JSON config
    ///////////////////////////////////
    StaticJsonDocument<512> doc;
    String json;

    doc["wifi"]["ssid"] = wifi.ssid;
    doc["wifi"]["password"] = wifi.password;
    doc["wifi"]["ap_ssid"] = wifi.ap_ssid;
    doc["wifi"]["ap_password"] = wifi.ap_password;
    doc["wifi"]["friendly_name"] = wifi.friendly_name;
    doc["wifi"]["mdns_name"] = wifi.mdns_name;
    doc["wifi"]["ap_enabled"] = wifi.ap_enabled;
    doc["software"]["spiffs"]["version"] = software.spiffs.version;
    doc["software"]["spiffs"]["build"] = software.spiffs.build;
    doc["software"]["spiffs"]["date"] = software.spiffs.date;
    doc["software"]["firmware"]["version"] = software.firmware.version;
    doc["software"]["firmware"]["build"] = software.firmware.build;
    doc["software"]["firmware"]["date"] = software.firmware.date;
    doc["ferrari_min"] = ferrari_min;
    doc["ferrari_max"] = ferrari_max;

    if (serializeJson(doc, json) == 0)
    {
        Serial.println("JSON serialization failed!");
    }

    return json;
    //////////////////////////////////
}

void soft_config_s::fromJSON(String json)
{
    // Convert JSON Config to struct
    ///////////////////////////////////
    StaticJsonDocument<512> doc;

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
    cpy(software.spiffs.version, doc["software"]["spiffs"]["version"]);
    cpy(software.spiffs.build, doc["software"]["spiffs"]["build"]);
    cpy(software.firmware.version, doc["software"]["firmware"]["version"]);
    cpy(software.firmware.build, doc["software"]["firmware"]["build"]);
    ferrari_min = doc["wifi"]["ferrari_min"];
    ferrari_max = doc["wifi"]["ferrari_max"];
    ///////////////////////////////////
}

void soft_config_s::store()
{
    // Save struct Config to Filesystem
    ///////////////////////////////////
    File f = LITTLEFS.open("/winder.conf", "w");
    if (f)
    {
        f.write((byte *)&soft, sizeof(soft));
        Serial.println("[Store] /winder.conf saved");
    }
    f.close();
    ///////////////////////////////////
}

void soft_config_s::backup()
{
    // Save JSON Config to EEPROM Backup
    ///////////////////////////////////
    LITTLEFS.remove("/winder.conf");
    prefs.begin("winder", false);
    String json = soft.asJSON();
    prefs.putString("config", json);
    Serial.println("[Store] Data Backup stored. Update can begin.");
    ///////////////////////////////////
}

bool soft_config_s::load()
{
    // Check if Filesystem is available
    ///////////////////////////////////
    byte buffer[1];
    const esp_partition_t *spiffs = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
    esp_partition_read(spiffs, 0, buffer, 1);
    if (buffer[0] == 0xff)
    {
        Serial.println("[Store] No Filesystem image found. Execution terminated.");
        return false;
    }

    if (!LITTLEFS.begin())
    {
        Serial.println("[Store] An Error has occurred while mounting LITTLEFS");
        return false;
    }
    ///////////////////////////////////

    // Load struct Config from Filesystem
    ///////////////////////////////////
    File f = LITTLEFS.open("/winder.conf", "r");
    if (f)
    {
        f.read((byte *)&soft, sizeof(soft));
        Serial.println("[Store] /winder.conf loaded from Filesystem.");
        f.close();
        return true;
    }
    else
    {
        Serial.println("[Store] First start after System update. No /winder.conf yet.");
    }

    ///////////////////////////////////

    // Load JSON Config from EEPROM Backup
    ///////////////////////////////////
    if (prefs.begin("winder", false) && prefs.isKey("config"))
    {
        String json = prefs.getString("config");
        Serial.println("[Store] Creating /winder.conf from Backup");
        Serial.println(json);
        soft.fromJSON(json);
    }
    else
    {
        Serial.println("[Store] No Data Backup found.");
    }
    prefs.end();
    ///////////////////////////////////

    Serial.println(soft.asJSON());
    return true;
}

void soft_config_s::loadBlynkCredentials()
{
    // Structure of Blynk Storage
    ///////////////////////////////////
    struct blynk_store_s
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
    } blynkStore;
    ///////////////////////////////////

    // Load Credentail struct from EEPROM
    ///////////////////////////////////
    if (prefs.begin("blynk", false) && prefs.getBytes("config", &blynkStore, sizeof(blynkStore)))
    {
        Serial.println("[Store] Found previous Blynk Config:");
        Serial.println(String(" => ") + blynkStore.wifiSSID);
        Serial.println(String(" => ") + blynkStore.wifiPass);
        strlcpy(soft.wifi.ssid, blynkStore.wifiSSID, sizeof(soft.wifi.ssid));
        strlcpy(soft.wifi.password, blynkStore.wifiPass, sizeof(soft.wifi.password));
    }
    else
    {
        Serial.println("[EEPROM] No Old Blynk Configuration found.");
    }
    prefs.end();
    ///////////////////////////////////
}

soft_config_s soft;
hard_config_s hard;
mode_e mode = STANDBY;
FastAccelStepperEngine engine;
connection_e connection = OFFLINE;