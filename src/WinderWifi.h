#pragma once

// Related
// System / External
#include <WiFi.h>
#include <timer.h>
// Selfmade
// Project
//#include <QiMachineWinder.h>
#include <QiMachineWinderConfiguration.h>

/**
 * @brief Current state of connecting to a wifi
 */
enum wifiConnectionMode_e {
    OFFLINE,     // No connection established
    CONNECTING,  // Trying to establish connection
    ONLINE,      // Active connection established
};

class QiMachineWinder;

/**
 * @brief Manages the connection to existing wifis and the creation of an (emergency) access point
 */
class WinderWifi {
   private:
    QiMachineWinder &_machine;           // Reference to the machine we manage the wifi for, needed for requesting the configuration
    TimerForMethods<WinderWifi> _timer;  // Timer for handeling the main loop of the wifi

    wifiConnectionMode_e _connectionMode = OFFLINE;
    const char *_ssid;                                // SSID of network
    const char *_password;                            // Password of network
    uint8_t _timeout = 15;                            // Timeout in seconds
    uint8_t _currentTimeout = 0;                      // Current timeout counter in seconds
    std::function<void()> _connectionChangeCallback;  // Callback for connection Change

    /**
     * @brief Invoke callback on connection Change
     *
     * @param newMode new connection-mode to be set
     */
    void changeMode(wifiConnectionMode_e newMode);

    /**
     * @brief Get software configuration of machine
     */
    configurationWifi_s &getConfiguration();

   public:
    /**
     * @brief Constructor
     *
     * @param winder Reference to machine to work with
     */
    WinderWifi(QiMachineWinder &winder);

    /**
     * @brief Start timers and general operation of wifi
     */
    void start();

    /**
     * @brief Setter method for callback for connection Change
     */
    void setConnectionChangeCallback(std::function<void()> newCallBack);

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
    void handle();

    // Getter-Method
    wifiConnectionMode_e getConnectionMode();
};
