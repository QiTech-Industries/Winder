/*
This example shows how to use Preferences (nvs) to store a
structure.  Note that the maximum size of a putBytes is 496K
or 97% of the nvs partition size.  nvs has signifcant overhead,
so should not be used for data that will change often.
The Storage has a lifespan of aproximately 100,000 reads/writes
The Namespace name must not be longer than 15 chars
*/

#include <EEPROM.h>
#include <Preferences.h>
Preferences preferences;

struct STORE {
    uint8_t puller_rps;
    uint32_t spool_mm_max;
    uint32_t spool_mm_min;
};

STORE store;

const STORE storeDefault = {
    .puller_rps = 1,
    .spool_mm_max = 4000,
    .spool_mm_min = 2000,
};

void config_load() {
    memset(&store, 0, sizeof(store));
    preferences.getBytes("config", &store, sizeof(store));
}

bool config_save() {
    preferences.putBytes("config", &store, sizeof(store));
    Serial.println("Configuration stored to flash");
    return true;
}

bool config_init() {
    preferences.begin("winder", false);
    config_load();
    return true;
}

void setup() {
    Serial.begin(115200);

    // Wait for any input from Serial monitor
    while (!Serial.available())
        ;

    config_init();

    // Should return 0 0 0 after flash erase with
    // C:\Users\<Your Username>\.platformio\penv\Scripts\pio.exe run --target erase
    // Should return values starting from second boot
    Serial.println(store.puller_rps);
    Serial.println(store.spool_mm_max);
    Serial.println(store.spool_mm_min);

    // Set values for write to the storage
    store = storeDefault;
    Serial.println(store.puller_rps);
    Serial.println(store.spool_mm_max);
    Serial.println(store.spool_mm_min);

    config_save();
}

void loop() {}
