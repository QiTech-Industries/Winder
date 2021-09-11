#include <global.h>

class Wifi
{
private:
    TimerForMethods<Wifi> timer;

    void loop()
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            DEBUG_PRINT(".");
        }
    }

public:
    Wifi() : timer(this, &Wifi::loop)
    {
        // Do not set WiFi.mode() or disconnect() as it causes esp_wifi_init to be called twice
        WiFi.setAutoReconnect(true);
        timer.setInterval(1000);
        WiFi.scanNetworks(true);
    }

    void connect(const char *ssid, const char *password)
    {
        // empty hostname causes mDNS name to be used (resulting in no .fritz.box)
        // Known error Reason: 2 - AUTH_EXPIRE see:
        // https://github.com/espressif/arduino-esp32/issues/2144

        WiFi.hostname("");
        WiFi.begin(ssid, password);
        timer.start();
        DEBUG_PRINTF("[Wifi] Connecting to WIFI: %s", ssid);
    }

    void disconnect()
    {
        WiFi.disconnect(true, true);
        timer.stop();
    }

    bool connected()
    {
        return WiFi.isConnected();
    }

    void createAP(const char *ssid, const char *password = (const char *)__null)
    {
        // softAPsetHostname() only works in pure AP mode
        WiFi.softAP(ssid, password);
        DEBUG_PRINTLN(String("[Wifi] Acces Point IP: ") + WiFi.softAPIP().toString());
    }

    String scan()
    {
        String json = "[";
        int n = WiFi.scanComplete();
        if (n == -2)
        {
            WiFi.scanNetworks(true);
        }
        else if (n)
        {
            for (int i = 0; i < n; ++i)
            {
                if (i)
                    json += ",";
                json += "{";
                json += "\"rssi\":" + String(WiFi.RSSI(i));
                json += ",\"ssid\":\"" + WiFi.SSID(i) + "\"";
                json += ",\"secure\":" + String(WiFi.encryptionType(i));
                json += "}";
            }
            WiFi.scanDelete();
            if (WiFi.scanComplete() == -2)
            {
                WiFi.scanNetworks(true);
            }
        }
        json += "]";
        return json;
    }

    IPAddress getAPIP()
    {
        return WiFi.softAPIP();
    }

    void destroyAP()
    {
        WiFi.softAPdisconnect(true);
    }
};