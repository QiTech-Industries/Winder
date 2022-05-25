#include <machine/QiWifi.h>

QiWifi::QiWifi() : _timer(this, &QiWifi::handle){};

void QiWifi::init() {
    _timer.setInterval(1000);
    _timer.start();
}

void QiWifi::connectAsync(const char* ssid, const char* password) {
    WiFi.hostname("");
    WiFi.setAutoReconnect(true);  // reconnect automatically after fail
    WiFi.begin(ssid, password);
    _connectStartedAt = millis();

    logPrint(LOG_LEVEL, INFO, "[Wifi] MAC Address: %s\n", WiFi.macAddress());
    logPrint(LOG_LEVEL, INFO, "[Wifi] Connecting to: %s\n", ssid);
};

void QiWifi::scanAsync() {
    // start scan if not yet started or failed previously
    if (WiFi.scanComplete() == -2) {
        logPrint(LOG_LEVEL, INFO, "[Wifi] Network scan started\n");
        WiFi.scanNetworks(true);
        _scanStartedAt = millis();
        return;
    }

    // ignore command and log warning is scan is already in progress
    if (WiFi.scanComplete() == -1) {
        logPrint(LOG_LEVEL, WARNING, "[Wifi] Network scan is still in progress\n");
        return;
    }
};

bool QiWifi::createAP(const char* ssid, const char* password) {
    if (WiFi.softAP(ssid, password)) {
        logPrint(LOG_LEVEL, INFO, "[Wifi] Access Point successfully created: %s\n", WiFi.softAPIP());
        _apStartedAt = millis();
        return true;
    }

    logPrint(LOG_LEVEL, ERROR, "[Wifi] AP SSID or password propably invalid\n");
    return false;
};

void QiWifi::onScanComplete(std::function<void(const char*)> cb) { _scanCompleteCb = cb; }

void QiWifi::onConnectSuccess(std::function<void()> cb) { _connectSuccessCb = cb; }

void QiWifi::onConnectFail(std::function<void()> cb) { _connectFailCb = cb; }

void QiWifi::handleScan() {
    // return immediately if scan was not started yet
    if (_scanStartedAt == 0) return;

    // Document to hold JSON network details
    StaticJsonDocument<512> doc;

    // log error if scan failed
    if (WiFi.scanComplete() == -2) {
        logPrint(LOG_LEVEL, ERROR, "[Wifi] Network scan failed\n");
        doc.createNestedArray("networks");
    }

    // generate network list if scan succeeded
    if (WiFi.scanComplete() >= 0) {
        const uint8_t networkCount = WiFi.scanComplete();
        logPrint(LOG_LEVEL, INFO, "[Wifi] Network scan completed, %i networks found\n", networkCount);

        // Assemble json with network information
        for (int i = 0; i < networkCount; ++i) {
            doc["networks"][i]["rssi"] = WiFi.RSSI(i);
            doc["networks"][i]["ssid"] = WiFi.SSID(i);
            doc["networks"][i]["secure"] = WiFi.encryptionType(i);
        }
    }

    doc["current"] = WiFi.isConnected() ? WiFi.SSID() : (char*)0;

    // reset Timeout and delete scan from RAM
    _scanStartedAt = 0;
    WiFi.scanDelete();

    _scanCompleteCb(doc.as<const char*>());
}

void QiWifi::handleConnect() {
    // return immediately if connect was not started yet
    if (_connectStartedAt == 0) return;

    wl_status_t wifiStatus = WiFi.status();

    // wait for wifi connection, fail, or timeout
    if (wifiStatus != WL_CONNECTED && wifiStatus != WL_NO_SSID_AVAIL && wifiStatus != WL_CONNECT_FAILED &&
        (millis() - _connectStartedAt) < CONNECT_TIMEOUT)
        return;

    // reset Timeout
    _connectStartedAt = 0;

    // log cause of error
    switch (wifiStatus) {
        case WL_CONNECTED:
            logPrint(LOG_LEVEL, INFO, "[WIFI] Connection established: %s\n", WiFi.localIP());
            break;
        case WL_NO_SSID_AVAIL:
            logPrint(LOG_LEVEL, ERROR, "[WIFI] Connection Failed AP not found.\n");
            break;
        case WL_CONNECT_FAILED:
            logPrint(LOG_LEVEL, ERROR, "[WIFI] Connection Failed, probably wrong ssid or password.\n");
            break;
        default:
            logPrint(LOG_LEVEL, ERROR, "[WIFI] Connection timed out.\n");
            WiFi.disconnect(); // make sure no connection attemps are made on timeout
            break;
    }

    // trigger success callback
    if (wifiStatus == WL_CONNECTED) {
        _connectSuccessCb();
        return;
    }

    // trigger connect fail callback
    _connectFailCb();
    if (_apStartedAt == 0) return;

    // Start emergency AP even if createAP() has never been called
    logPrint(LOG_LEVEL, ERROR, "[Wifi] Starting emergency AP\n");
    createAP(EMERGENCY_AP_NAME, EMERGENCY_AP_PASSWORD);
}

void QiWifi::handle() {
    handleScan();
    handleConnect();
};
