#pragma once

#include <ArduinoJson.h>
#include <WiFi.h>
#include <logger/logging.h>
#include <stdint.h>
#include <timerManager.h>

const loggingLevel_e LOG_LEVEL = INFO;
const uint16_t CONNECT_TIMEOUT = 15000;
const char* EMERGENCY_AP_NAME = "Jarvis Winder";
const char* EMERGENCY_AP_PASSWORD = "";

/**
 * @brief Manages the connection to existing wifis and the creation of an (emergency) access point
 */
class QiWifi {
   private:
    TimerForMethods<QiWifi> _timer;  // Timer for handeling the main loop of the wifi

    std::function<void(const char *)> _scanCompleteCb;  // Callback for connection Change
    std::function<void()> _connectSuccessCb;            // Callback for connection Change
    std::function<void()> _connectFailCb;               // Callback for connection Change
    uint16_t _scanStartedAt = 0;
    uint16_t _connectStartedAt = 0;
    uint16_t _apStartedAt = 0;

    /**
     * @brief Check if network scan finished and act on result
     *
     */
    void handleScan();

    /**
     * @brief Check if wifi connection established and act on result
     * Create emergency AP if not connection can be established
     *
     */
    void handleConnect();

   public:
    /**
     * @brief Constructor
     *
     */
    QiWifi();

    /**
     * @brief Start timers and general operation of wifi
     */
    void init();

    /**
     * @brief Start async connect to Wifi Network
     *
     * @param ssid SSID of network
     * @param password Password of network
     */
    void connectAsync(const char *ssid, const char *password);

    /**
     * @brief Start async scan for available networks
     *
     */
    void scanAsync();

    /**
     * @brief Make ESP send out its own Access Point
     *
     * @param ssid ssid of the access point
     * @param password password of the access point
     */
    bool createAP(const char *ssid, const char *password = (const char *)__null);

    /**
     * @brief Register Callback handler for scan complete event
     *
     * @param cb Callback function
     */
    void onScanComplete(std::function<void(const char *)> cb);

    /**
     * @brief Register Callback handler for connect fail event
     *
     * @param cb Callback function
     */
    void onConnectFail(std::function<void()> cb);

    /**
     * @brief Register Callback handler for connect success event
     *
     * @param cb Callback function
     */
    void onConnectSuccess(std::function<void()> cb);

    /**
     * @brief Called repeatedly by timer, calls handleConnect and handleScan
     */
    void handle();
};
