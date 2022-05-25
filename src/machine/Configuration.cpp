#include <machine/Configuration.h>

JsonObject winderSoftwareConfiguration_s::toJSON() {
    // Convert struct to JSON config
    StaticJsonDocument<512> doc;

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

    return doc.as<JsonObject>();
}

void winderSoftwareConfiguration_s::fromJSON(char *json) {
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

void winderSoftwareConfiguration_s::store() {
    // TODO: load file location from config
    File f = LITTLEFS.open("/winder.conf", "w");
    if (f) {
        f.write((byte *)this, sizeof(winderSoftwareConfiguration_s));
        logPrint(LOG_LEVEL, INFO, "[Store] /winder.conf saved\n");
    }
    f.close();
}

void winderSoftwareConfiguration_s::backup() {
    // TODO: load file location from config
    LITTLEFS.remove("/winder.conf");
    prefs.begin("winder", false);
    char *json;
    serializeJson(this->toJSON(), json);
    prefs.putBytes("config", json, 512);
    logPrint(LOG_LEVEL, INFO, "[Store] Data Backup stored. Update can begin.\n");
}

void winderSoftwareConfiguration_s::loadBlynkCredentials() {
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
        logPrint(LOG_LEVEL, INFO, "[Store] Found previous Blynk Config:\n => %c\n => %c\n", blynkStore.wifiSSID, blynkStore.wifiPass);
        strlcpy(this->wifi.ssid, blynkStore.wifiSSID, sizeof(this->wifi.ssid));
        strlcpy(this->wifi.password, blynkStore.wifiPass, sizeof(this->wifi.password));
    } else {
        logPrint(LOG_LEVEL, INFO, "[Store] No Old Blynk Configuration found.\n");
    }
    prefs.end();
}

bool winderSoftwareConfiguration_s::load() {
    winderSoftwareConfiguration_s::loadBlynkCredentials();

    // Check whether filesystem is available
    byte buffer[1];
    const esp_partition_t *spiffs = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
    esp_partition_read(spiffs, 0, buffer, 1);
    if (buffer[0] == 0xff) {
        logPrint(LOG_LEVEL, ERROR, "[Store] No Filesystem image found. Only API access possible.\n");
        return false;
    }
    if (!LITTLEFS.begin()) {
        logPrint(LOG_LEVEL, ERROR, "[Store] An Error has occurred while mounting LITTLEFS\n");
        return false;
    }

    // Load configuration from filesystem
    File f = LITTLEFS.open("/winder.conf", "r");
    if (f) {
        f.read((byte *)this, sizeof(winderSoftwareConfiguration_s));
        logPrint(LOG_LEVEL, INFO, "[Store] /winder.conf loaded from Filesystem.\n");
        f.close();
        return true;
    } else {
        logPrint(LOG_LEVEL, INFO, "[Store] First start after System update. No /winder.conf yet.\n");
    }

    // Load JSON Config from EEPROM Backup
    if (prefs.begin("winder", false) && prefs.isKey("config")) {
        char buffer[512];
        prefs.getBytes("config", buffer, 512);
        logPrint(LOG_LEVEL, INFO, "[Store] Loading Backup into Config\n");
        this->fromJSON(buffer);
    } else {
        logPrint(LOG_LEVEL, INFO, "[Store] No Data Backup found.\n");
    }
    prefs.end();

    return true;
}

bool winderConfiguration_s::isValid() {
    // Check pin-configuration
    McValidator pinValidator = McValidator();
    int8_t digital_pins[12] = {hard.motors.spool.pins.cs,   hard.motors.spool.pins.dir,    hard.motors.spool.pins.en,
                               hard.motors.spool.pins.step, hard.motors.ferrari.pins.cs,   hard.motors.ferrari.pins.dir,
                               hard.motors.ferrari.pins.en, hard.motors.ferrari.pins.step, hard.motors.puller.pins.cs,
                               hard.motors.puller.pins.dir, hard.motors.puller.pins.en,    hard.motors.puller.pins.step};

    for (int8_t i; i < 12; i++) {
        if (!pinValidator.isDigitalPinValid(digital_pins[i])) {
            logPrint(LOG_LEVEL, ERROR, "[Store] Invalid Stepper Pins configured\n");
            return false;
        }
    }

    return true;
}
