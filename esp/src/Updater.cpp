#include <global.h>

class Updater
{
private:
    String overTheAirURL = "http://update.qitech.de/";
    const char *headerKeys[4] = {"X-Update", "X-Build", "X-Version", "date"};
    TimerForMethods<Updater> timer;
    HTTPClient http;

public:
    Updater() : timer(this, &Updater::check)
    {
        http.collectHeaders(headerKeys, 4);
    }

    void restartMCU()
    {
        ESP.restart();
        while (1)
        {
        };
    }

    void setInterval(uint16_t interval)
    {
        timer.setInterval(interval);
        timer.start();
    }

    void check()
    {
        if (!WiFi.isConnected() || Update.isRunning() || mode != STANDBY)
        {
            return;
        }

        http.begin(overTheAirURL + WiFi.macAddress());
        http.GET();

        String XUpdate = http.header(headerKeys[0]);
        String XBuild = http.header(headerKeys[1]);
        String XVersion = http.header(headerKeys[2]);
        String date = http.header(headerKeys[3]);
        int partition;

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

        http.setURL(overTheAirURL + "success/" + WiFi.macAddress());
        DEBUG_PRINTLN(soft.asJSON());

        if (http.GET() == HTTP_CODE_NO_CONTENT)
        {
            DEBUG_PRINTLN("[Updater] Update successfully completed. Rebooting.");
            restartMCU();
        }
    }
};