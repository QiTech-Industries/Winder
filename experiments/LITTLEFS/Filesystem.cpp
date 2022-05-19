#include <global.h>

// LITTLEFS.begin() can not be called in constructor because runtime is not ready there
class Filesystem {
   public:
    Filesystem() {}

    bool init() {
        byte buffer[1];
        const esp_partition_t *spiffs = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
        esp_partition_read(spiffs, 0, buffer, 1);
        if (buffer[0] == 0xff) {
            DEBUG_PRINTLN("[Filesystem] No Filesystem image found. Execution terminated.");
            return false;
        }

        if (!LITTLEFS.begin()) {
            DEBUG_PRINTLN("[Filesystem] An Error has occurred while mounting LITTLEFS");
            return false;
        }

        return true;
    }

    // read file from littlefs
    bool read(const char *filename, byte *config, uint16_t len) {
        File f = LITTLEFS.open(filename, "r");
        if (f) {
            f.read(config, len);
            f.close();
            return true;
        }
        f.close();
        return false;
    }

    // load global config file at program start
    void loadConfig() {
        if (!read("winder.conf", (byte *)&soft, sizeof(soft))) {
            DEBUG_PRINTLN("[Filesystem] First start after System update. No winder.conf yet.");
            DEBUG_PRINTLN("[Filesystem] Creating new winder.conf");
        } else {
            DEBUG_PRINTLN("[Filesystem] winder.conf loaded from Filesystem.");
        }
    }
};
