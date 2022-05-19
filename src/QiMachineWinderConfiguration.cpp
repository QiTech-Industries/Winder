// Related
#include <QiMachineWinderConfiguration.h>
// System / External
#include <ArduinoJson.h>
#include <LITTLEFS.h>
#include <Preferences.h>
// Selfmade
#include "controller/stepper/StepperTest.h"
// Project
#include <QiMachineWinderConfiguration.h>

Preferences prefs;

String configurationMachineWinderSoftware_s::asJSON() {
    // Convert struct to JSON config
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

    if (serializeJson(doc, json) == 0) {
        Serial.println("[Store] JSON serialization failed!");
    }

    return json;
}

void configurationMachineWinderSoftware_s::fromJSON(char *json) {
    StaticJsonDocument<512> doc;

    deserializeJson(doc, json);
    // TODO: Abort on error?

    strcpy(wifi.ssid, doc["wifi"]["ssid"]);
    strcpy(wifi.password, doc["wifi"]["password"]);
    strcpy(wifi.ap_ssid, doc["wifi"]["ap_ssid"]);
    strcpy(wifi.ap_password, doc["wifi"]["ap_password"]);
    strcpy(wifi.friendly_name, doc["wifi"]["friendly_name"]);
    strcpy(wifi.mdns_name, doc["wifi"]["mdns_name"]);
    wifi.ap_enabled = doc["wifi"]["ap_enabled"];

    strcpy(software.spiffs.version, doc["software"]["spiffs"]["version"]);
    strcpy(software.spiffs.build, doc["software"]["spiffs"]["build"]);
    strcpy(software.spiffs.date, doc["software"]["spiffs"]["date"]);
    strcpy(software.firmware.version, doc["software"]["firmware"]["version"]);
    strcpy(software.firmware.build, doc["software"]["firmware"]["build"]);
    strcpy(software.firmware.date, doc["software"]["firmware"]["date"]);

    ferrari_min = doc["ferrari_min"];
    ferrari_max = doc["ferrari_max"];
}

void configurationMachineWinderSoftware_s::store() {
    File f = LITTLEFS.open("/winder.conf", "w");
    if (f) {
        f.write((byte *)this, sizeof(configurationMachineWinderSoftware_s));
        Serial.println("[Store] /winder.conf saved");
    }
    f.close();
}

void configurationMachineWinderSoftware_s::backup() {
    LITTLEFS.remove("/winder.conf");
    prefs.begin("winder", false);
    String json = this->asJSON();
    prefs.putBytes("config", json.c_str(), 512);
    Serial.println("[Store] Data Backup stored. Update can begin.");
}

void configurationMachineWinderSoftware_s::loadBlynkCredentials() {
    // Structure of Blynk Storage
    struct blynk_store_s {
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

    // Load Credentail struct from EEPROM
    if (prefs.begin("blynk", false) && prefs.getBytes("config", &blynkStore, sizeof(blynkStore))) {
        Serial.println("[Store] Found previous Blynk Config:");
        Serial.println(String(" => ") + blynkStore.wifiSSID);
        Serial.println(String(" => ") + blynkStore.wifiPass);
        strlcpy(this->wifi.ssid, blynkStore.wifiSSID, sizeof(this->wifi.ssid));
        strlcpy(this->wifi.password, blynkStore.wifiPass, sizeof(this->wifi.password));
    } else {
        Serial.println("[EEPROM] No Old Blynk Configuration found.");
    }
    prefs.end();
}

bool configurationMachineWinderSoftware_s::load() {
    configurationMachineWinderSoftware_s::loadBlynkCredentials();

    // Check whether filesystem is available
    byte buffer[1];
    const esp_partition_t *spiffs = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
    esp_partition_read(spiffs, 0, buffer, 1);
    if (buffer[0] == 0xff) {
        Serial.println("[Store] No Filesystem image found. Only API access possible.");
        return false;
    }
    if (!LITTLEFS.begin()) {
        Serial.println("[Store] An Error has occurred while mounting LITTLEFS");
        return false;
    }

    // Load configuration from filesystem
    File f = LITTLEFS.open("/winder.conf", "r");
    if (f) {
        f.read((byte *)this, sizeof(configurationMachineWinderSoftware_s));
        Serial.println("[Store] /winder.conf loaded from Filesystem.");
        f.close();
        return true;
    } else {
        Serial.println("[Store] First start after System update. No /winder.conf yet.");
    }

    // Load JSON Config from EEPROM Backup
    if (prefs.begin("winder", false) && prefs.isKey("config")) {
        char buffer[512];
        prefs.getBytes("config", buffer, 512);
        Serial.println("[Store] Loading Backup into Config");
        this->fromJSON(buffer);
    } else {
        Serial.println("[Store] No Data Backup found.");
    }
    prefs.end();

    return true;
}
