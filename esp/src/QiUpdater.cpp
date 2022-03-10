// Related
#include <QiUpdater.h>
// System / External
#include <string>
#include <stdint.h>
#include <timer.h>
#include <HTTPClient.h>
#include <Update.h> // Update Frimware and Interface
// Selfmade
// Project
#include <QiMachineWinder.h>
#include <QiMachineWinderConfiguration.h>

// TODO: Replace with proper debugging/logging
#ifdef DEBUG_WINDER
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x...)
#endif

QiUpdater::QiUpdater(QiMachineWinder& winder) : _machine(winder), timer(this, &QiUpdater::handle) {
    http.collectHeaders(headerKeys, 4);
}

configurationMachineWinderSoftware_s& QiUpdater::getConfiguration(){
    return _machine.getConfigurationSoft();
}

void QiUpdater::restartMCU() {
    ESP.restart();
    while (true) {}; // Wait for microcontroller to finally die
}

void QiUpdater::setInterval(uint16_t interval) {
    timer.setInterval(interval);
    timer.start();
}

void QiUpdater::handle() {
    // Check whether we are ready for an update
    if (!WiFi.isConnected() || Update.isRunning() || (_machine.getCurrentMode() != OPERATE_OFF && _machine.getCurrentMode() != OPERATE_STANDBY)) {
        return;
    }

    // Check for and download new update
    http.begin(overTheAirURL + WiFi.macAddress());
    http.GET();

    String XUpdate = http.header(headerKeys[0]);
    String XBuild = http.header(headerKeys[1]);
    String XVersion = http.header(headerKeys[2]);
    String date = http.header(headerKeys[3]);
    int partition; // Identifier which partition has to be updated and thus rewritten

    if (XUpdate == "spiffs") {
        partition = U_SPIFFS;
        strcpy(getConfiguration().software.spiffs.version, XVersion.c_str());
        strcpy(getConfiguration().software.spiffs.build, XBuild.c_str());
        strcpy(getConfiguration().software.spiffs.date, date.substring(0, 16).c_str());
    } else if (XUpdate == "firmware") {
        partition = U_FLASH;
        strcpy(getConfiguration().software.firmware.version, XVersion.c_str());
        strcpy(getConfiguration().software.firmware.build, XBuild.c_str());
        strcpy(getConfiguration().software.firmware.date, date.substring(0, 16).c_str());
    } else {
        DEBUG_PRINTLN("[Updater] No new Update available");
        return;
    }

    // Try to install new update
    int contentLength = http.getSize();
    if (contentLength <= 0) {
        DEBUG_PRINTLN("[Updater] OTA Content-Length not defined");
        return;
    }
    if (!Update.begin(contentLength, partition)) {
        DEBUG_PRINTLN("[Updater] Not enough space to begin OTA");
        return;
    }
    
    getConfiguration().backup();
    DEBUG_PRINTLN("[Updater] Starting update of " + XUpdate);

    Client &client = http.getStream();
    Update.writeStream(client);

    if (!Update.end()) {
        DEBUG_PRINTLN("[Updater] Error #" + String(Update.getError()));
        return;
    }
    if (!Update.isFinished()) {
        DEBUG_PRINTLN("[Updater] Update failed.");
        return;
    }

    // Update succesful, tell server
    http.setURL(overTheAirURL + "success/" + WiFi.macAddress());
    DEBUG_PRINTLN(getConfiguration().asJSON());

    // Do NOT restart to avoid interrupting the users real life workflow
    /*
    if (http.GET() == HTTP_CODE_NO_CONTENT) {
        DEBUG_PRINTLN("[Updater] Update successfully completed. Rebooting.");
        restartMCU();
    }
    */
}