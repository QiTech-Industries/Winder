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

#include <QiUpdater.h>

#include <string>
#include <stdint.h>
#include <timer.h>
#include <HTTPClient.h>
#include <Update.h> // Update Frimware and Interface

#include <store.h>

QiUpdater::QiUpdater() : timer(this, &QiUpdater::check)
{
    http.collectHeaders(headerKeys, 4);
}

void QiUpdater::restartMCU()
{
    ESP.restart();
    while (true) {}; // Wait for microcontroller to finally die
}

void QiUpdater::setInterval(uint16_t interval)
{
    timer.setInterval(interval);
    timer.start();
}

void QiUpdater::check()
{
    if (!WiFi.isConnected() || Update.isRunning() || mode != STANDBY)
    {
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

    if (XUpdate == "spiffs")
    {
        partition = U_SPIFFS;
        strcpy(soft.software.spiffs.version, XVersion.c_str());
        strcpy(soft.software.spiffs.build, XBuild.c_str());
        strcpy(soft.software.spiffs.date, date.substring(0, 16).c_str());
    }
    else if (XUpdate == "firmware")
    {
        partition = U_FLASH;
        strcpy(soft.software.firmware.version, XVersion.c_str());
        strcpy(soft.software.firmware.build, XBuild.c_str());
        strcpy(soft.software.firmware.date, date.substring(0, 16).c_str());
    }
    else
    {
        DEBUG_PRINTLN("[Updater] No new Update available");
        return;
    }

    // Try to install new update
    int contentLength = http.getSize();
    if (contentLength <= 0)
    {
        DEBUG_PRINTLN("[Updater] OTA Content-Length not defined");
        return;
    }
    if (!Update.begin(contentLength, partition))
    {
        DEBUG_PRINTLN("[Updater] Not enough space to begin OTA");
        return;
    }
    
    soft.backup();
    DEBUG_PRINTLN("[Updater] Starting update of " + XUpdate);

    Client &client = http.getStream();
    Update.writeStream(client);

    if (!Update.end())
    {
        DEBUG_PRINTLN("[Updater] Error #" + String(Update.getError()));
        return;
    }
    if (!Update.isFinished())
    {
        DEBUG_PRINTLN("[Updater] Update failed.");
        return;
    }

    // Update succesful, tell server and restart
    http.setURL(overTheAirURL + "success/" + WiFi.macAddress());
    DEBUG_PRINTLN(soft.asJSON());

    if (http.GET() == HTTP_CODE_NO_CONTENT)
    {
        DEBUG_PRINTLN("[Updater] Update successfully completed. Rebooting.");
        restartMCU();
    }
}