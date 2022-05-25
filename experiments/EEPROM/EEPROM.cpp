#include <global.h>

class EEPROM {
   private:
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

    Preferences prefs;

   public:
    EEPROM() {}

    void loadBlynkConfig() {
        if (prefs.begin("blynk", false) && prefs.getBytes("config", &blynkStore, sizeof(blynkStore))) {
            DEBUG_PRINTLN("[EEPROM] Found previous Blynk Config:");
            DEBUG_PRINTLN(String(" => ") + blynkStore.wifiSSID);
            DEBUG_PRINTLN(String(" => ") + blynkStore.wifiPass);
            strlcpy(soft.wifi.ssid, blynkStore.wifiSSID, sizeof(soft.wifi.ssid));
            strlcpy(soft.wifi.password, blynkStore.wifiPass, sizeof(soft.wifi.password));
        } else {
            DEBUG_PRINTLN("[EEPROM] No Old Blynk Configuration found.");
        }
        prefs.end();
    }

    void readJSON() {
        if (prefs.begin("winder", false) && prefs.isKey("config")) {
            String json = prefs.getString("config");
            DEBUG_PRINTLN("[EEPROM] Data Backup found:");
            DEBUG_PRINTLN(json);
            // soft.fromJSON(json);
        } else {
            DEBUG_PRINTLN("[EEPROM] No Data Backup found.");
        }
        prefs.end();
    }

    void writeJSON() {
        prefs.begin("winder", false);
        String json = soft.asJSON();
        DEBUG_PRINTLN("[EEPROM] Data Backup stored. Update can begin.");
        prefs.putString("config", json);
    }

    void init() {
        loadBlynkConfig();
        readJSON();
    }
};
