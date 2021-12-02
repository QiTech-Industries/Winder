#include <global.h>

class Wifi
{
private:
    TimerForMethods<Wifi> _timer;
    const char *_ssid;
    const char *_password;
    uint8_t _timeout = 15;
    uint8_t _currentTimeout = 0;
    std::function<void()> _cb;

    // Invoke callback on connection Change
    ///////////////////////////////////
    void changeMode(connection_e to)
    {
        connection = to;
        if (_cb)
            _cb();
    }
    ///////////////////////////////////

public:
    Wifi() : _timer(this, &Wifi::loop)
    {
        WiFi.scanNetworks(true);
        _timer.setInterval(1000);
        _timer.start();
    }

    // Set callback for connection Change
    ///////////////////////////////////
    void conChange(std::function<void()> cb)
    {
        _cb = cb;
    }
    ///////////////////////////////////

    // Connect to Wifi Network
    ///////////////////////////////////
    void connect(const char *ssid, const char *password)
    {
        _ssid = ssid;
        _password = password;

        WiFi.hostname("");
        WiFi.begin(ssid, password);

        DEBUG_PRINTLN(String("[Wifi] MAC Address: ") + WiFi.macAddress());
        DEBUG_PRINTF("[Wifi] Connecting to WIFI: %s\n", ssid);

        changeMode(CONNECTING);
    }
    ///////////////////////////////////

    // Create Access Point
    ///////////////////////////////////
    void createAP(const char *ssid, const char *password = (const char *)__null)
    {
        WiFi.softAP(ssid, password);
        DEBUG_PRINTLN(String("[Wifi] Acces Point IP: ") + WiFi.softAPIP().toString());
    }
    ///////////////////////////////////

    // Scan for available networks
    ///////////////////////////////////
    String scan()
    {
        int n = WiFi.scanComplete();
        if (n == -2)
        {
            WiFi.scanNetworks(true);
            return String("{\"networks\": []}");
        }

        String json = "{\"networks\":[";
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
        json += "],\"current\":" + (WiFi.isConnected() ? ("\"" + String(soft.wifi.ssid) + "\"") : "null") + "}";
        return json;
    }
    ///////////////////////////////////

    void loop()
    {
        // Save Wifi Credentials on connection
        ///////////////////////////////////
        if (connection != ONLINE && WiFi.isConnected())
        {
            DEBUG_PRINTLN("[Wifi] Wifi successfully connected");

            strcpy(soft.wifi.ssid, _ssid);
            strcpy(soft.wifi.password, _password);

            _currentTimeout = 0;

            changeMode(ONLINE);
        }
        ///////////////////////////////////

        // Start AP if connection failed
        ///////////////////////////////////
        else if (connection == CONNECTING && !WiFi.isConnected())
        {
            _currentTimeout++;
            if (_currentTimeout >= _timeout)
            {
                DEBUG_PRINTLN("[Wifi] Could not connect to Wifi.");
                changeMode(OFFLINE);
                _currentTimeout = 0;

                // connect to previous network if new failed
                if (soft.wifi.ssid != _ssid)
                {
                    connect(soft.wifi.ssid, soft.wifi.password);
                }
                else
                {
                    WiFi.disconnect(true, false);
                }

                // Start emergency Access Point
                if (!soft.wifi.ap_enabled)
                {
                    DEBUG_PRINTLN("[Wifi] Starting in AP mode instead.");
                    soft.wifi.ap_enabled = true;
                    createAP("Winder", "");
                }
            }
        }
        ///////////////////////////////////

        else if (connection == ONLINE && !WiFi.isConnected())
        {
            DEBUG_PRINTLN("[Wifi] Network disconnected");
            changeMode(OFFLINE);
        }
    }
};