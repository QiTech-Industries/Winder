#include <global.h>

class Wifi
{
public:
    Wifi()
    {
        // Do not set WiFi.mode() or disconnect() as it causes esp_wifi_init to be called twice
        WiFi.scanNetworks(true);
    }

    void connect(const char *ssid, const char *password)
    {
        // empty hostname causes mDNS name to be used (resulting in no .fritz.box)
        // Known error Reason: 2 - AUTH_EXPIRE see:
        // https://github.com/espressif/arduino-esp32/issues/2144

        WiFi.hostname("");
        WiFi.begin(ssid, password);
        DEBUG_PRINTLN(String("[Wifi] MAC Address: ") + WiFi.macAddress());
        DEBUG_PRINTF("[Wifi] Connecting to WIFI: %s\n", ssid);
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

    void updateStatus()
    {
        if (connection == OFFLINE && WiFi.isConnected())
        {
            DEBUG_PRINTLN("[Wifi] Wifi successfully connected");
            //server.emit("connect", String("\"connected\""));
            connection = ONLINE;
        }
        else if (connection == CONNECTING && WiFi.status() > WL_CONNECTED)
        {
            DEBUG_PRINTLN("Wifi connection failed");
            //server.emit("connect", String("\"failed\""));
            connection = OFFLINE;
        }
        else if (connection == ONLINE && !WiFi.isConnected())
        {
            connection = OFFLINE;
        }
    }
};