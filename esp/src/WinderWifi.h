#pragma once
#include <timer.h>

#include <WiFi.h>

#include <store.h>

/**
 * @brief Manages the connection to existing wifis and the creation of an (emergency) access point
 */
class WinderWifi
{
private:
    TimerForMethods<WinderWifi> _timer; // Timer for handeling the main loop of the wifi
    const char *_ssid; // SSID of network
    const char *_password; // Password of network
    uint8_t _timeout = 15; // Timeout in seconds
    uint8_t _currentTimeout = 0; // Current timeout counter in seconds
    std::function<void()> _cb; // Callback for connection Change

    /**
     * @brief Invoke callback on connection Change
     * 
     * @param to TODO: Rename and comment parameter
     */
    void changeMode(connection_e to);

public:
    // Default Constructor
    WinderWifi();

    /**
     * @brief Setter method for callback for connection Change
     */
    void conChange(std::function<void()> cb); // TODO: Rename to setConnectionChange() / setCb()...

    /**
     * @brief Connect to Wifi Network
     * 
     * @param ssid SSID of network
     * @param password Password of network
     */
    void connect(const char *ssid, const char *password);

    /**
     * @brief Create Access Point
     * 
     * @param ssid ssid of the access point
     * @param password password of the access point
     */
    void createAP(const char *ssid, const char *password = (const char *)__null);

    /**
     * @brief Scan for available networks
     * 
     * @return String a json-formatted list of available
     */
    String scan();

    /**
     * @brief Called repeatedly by timer, handles (re-)connecting to existing wifis or creating an internal (emergency) wifi
     */
    void loop(); // TODO: Rename to handle() ?
};