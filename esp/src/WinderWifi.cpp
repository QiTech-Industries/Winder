// Related
#include <WinderWifi.h>
// System / External
#include <WiFi.h>
// Selfmade
// Project
#include <QiMachineWinder.h>

// TODO: replace with proper logging
#ifdef DEBUG_WINDER
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x...)
#endif

void WinderWifi::changeMode(wifiConnectionMode_e newMode) {
    _connectionMode = newMode; // TODO: Remove access on global var
    if (_connectionChangeCallback) _connectionChangeCallback();
};

WinderWifi::WinderWifi(QiMachineWinder& winder) : _machine(winder), _timer(this, &WinderWifi::handle) {};

void WinderWifi::start(){
    WiFi.scanNetworks(true);
    _timer.setInterval(1000);
    _timer.start();
}

configurationWifi_s& WinderWifi::getConfiguration(){
    return _machine.getConfigurationSoft().wifi; // TODO: replace with non-global code
}

void WinderWifi::setConnectionChangeCallback(std::function<void()> newCallBack) {
    _connectionChangeCallback = newCallBack;
};

void WinderWifi::connect(const char *ssid, const char *password) {
    _ssid = ssid;
    _password = password;

    WiFi.hostname("");
    WiFi.begin(ssid, password);

    DEBUG_PRINTLN(String("[Wifi] MAC Address: ") + WiFi.macAddress());
    DEBUG_PRINTF("[Wifi] Connecting to: %s\n", ssid);

    changeMode(CONNECTING);
};

void WinderWifi::createAP(const char* ssid, const char* password) {
    WiFi.softAP(ssid, password);
    DEBUG_PRINTLN(String("[Wifi] Created Access Point: ") + WiFi.softAPIP().toString());
};

String WinderWifi::scan() {
    int numberOfNetworks = WiFi.scanComplete();
    if (numberOfNetworks == -2) { // Scan not triggered
        WiFi.scanNetworks(true);
        return String("{\"networks\": []}");
    }

    // Assemble json-formatted answer
    // TODO: Creating a json with string-concatenation is not exactly efficient / beautiful
    String json = "{\"networks\":[";
    for (int i = 0; i < numberOfNetworks; ++i) {
        if (i) json += ",";
        json += "{";
        json += "\"rssi\":" + String(WiFi.RSSI(i));
        json += ",\"ssid\":\"" + WiFi.SSID(i) + "\"";
        json += ",\"secure\":" + String(WiFi.encryptionType(i));
        json += "}";
    }
    WiFi.scanDelete();
    if (WiFi.scanComplete() == -2) WiFi.scanNetworks(true);
    json += "],\"current\":" + (WiFi.isConnected() ? ("\"" + String(getConfiguration().ssid) + "\"") : "null") + "}";

    return json;
};

void WinderWifi::handle() {
    if (_connectionMode != ONLINE && WiFi.isConnected()) {
        // Save Wifi Credentials on connection
        DEBUG_PRINT("[Wifi] Wifi successfully connected as ");
        Serial.println(WiFi.localIP());

        strcpy(getConfiguration().ssid, _ssid);
        strcpy(getConfiguration().password, _password);
        
        _currentTimeout = 0;

        changeMode(ONLINE);
    } else if (_connectionMode == CONNECTING && !WiFi.isConnected()) {
        // Start AP if connection failed
        _currentTimeout++;
        if (_currentTimeout >= _timeout) {
            DEBUG_PRINTLN("[Wifi] Could not connect to Wifi.");
            changeMode(OFFLINE);
            _currentTimeout = 0;

            // connect to previous network if new failed
            if (getConfiguration().ssid != _ssid) {
                connect(getConfiguration().ssid, getConfiguration().password);
            } else {
                WiFi.disconnect(true, false);
            }

            // Start emergency Access Point
            if (!getConfiguration().ap_enabled) {
                DEBUG_PRINTLN("[Wifi] Starting in AP mode instead.");
                getConfiguration().ap_enabled = true;
                createAP("Winder", ""); // TODO: Hardcoded name/password for emergency access point
            }
        }
    } else if (_connectionMode == ONLINE && !WiFi.isConnected()) {
        DEBUG_PRINTLN("[Wifi] Network disconnected");
        changeMode(OFFLINE);
    }
};

wifiConnectionMode_e WinderWifi::getConnectionMode(){
    return _connectionMode;
}
