#include <global.h>

class Updater
{
private:
    String overTheAirURL = "http://192.168.178.42:3000/update";
    String interfaceVersion = "0.0.0";
    String firmwareVersion = "0.0.0";
    const char *headerKeys[1] = {"X-Update"};
    TimerForMethods<Updater> timer;
    HTTPClient http;

public:
    Updater() : timer(this, &Updater::check)
    {
        http.collectHeaders(headerKeys, 1);
    }

    void restartMCU()
    {
        ESP.restart();
        while (1)
        {
        };
    }

    void setInterval(uint16_t)
    {
        timer.setInterval(1000);
        timer.start();
    }
    void check()
    {
        if (!WiFi.isConnected() || Update.isRunning())
        {
            return;
        }

        DEBUG_PRINTLN("[Updater] Checking for Update...");

        http.begin(overTheAirURL + "?spiffs=" + interfaceVersion + "&firmware=" + firmwareVersion);

        int httpCode = http.GET();

        if (httpCode != HTTP_CODE_OK)
        {
            DEBUG_PRINTLN("[Updater] Error connecting to Update Server");
            return;
        }

        String XUpdate = http.header(headerKeys[0]);
        int partition;
        if (XUpdate == "spiffs")
        {
            partition = U_SPIFFS;
        }
        else if (XUpdate == "firmware")
        {
            partition = U_FLASH;
        }
        else
        {
            DEBUG_PRINTLN("[Updater] No new Update available");
            return;
        }

        int contentLength = http.getSize();
        if (contentLength <= 0)
        {
            DEBUG_PRINTLN("[Updater] Content-Length not defined");
            return;
        }

        bool canBegin = Update.begin(contentLength, partition);
        if (!canBegin)
        {
            DEBUG_PRINTLN("[Updater] Not enough space to begin OTA");
            return;
        }

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

        DEBUG_PRINTLN("[Updater] Update successfully completed. Rebooting.");
        restartMCU();
    }
};