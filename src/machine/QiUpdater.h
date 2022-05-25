#pragma once

#include <HTTPClient.h>
#include <Update.h>
#include <logger/logging.h>
#include <stdint.h>
#include <timer.h>

#include <string>

const loggingLevel_e LOG_LEVEL = INFO;
const uint16_t UPDATE_INTERVAL = 30000;

// We are using String here, as HTTPClient does not allow char arrays
// Url to check for updates
const String OTA_URL = "http://update.qitech.de/";

// HTTP-Headers needed for extracting response headers, needs to be an array for use as http-client parameter
const char* UPDATE_HEADERS[4] = {"X-Update", "X-Build", "X-Version", "date"};

/**
 * @brief Checks for updates frequently and downloads/installs them when available
 */
class QiUpdater {
   private:
    TimerForMethods<QiUpdater> timer;  // Timer for main looping of handeling
    HTTPClient http;                   // Client for requesting data over the network
    String _macAddress;
    std::function<bool()> _beforeUpdateCb;
    std::function<void(String, String, String, String)> _updateSuccessCb;

    /**
     * @brief Repeatedly called by timer, checks for new updates, downloads / installs them when available
     */
    void handle();

   public:
    /**
     * @brief Construct a new Qi Updater object
     *
     * @param macAddress Mac address of wifi device
     */
    QiUpdater(const String macAddress);

    /**
     * @brief Restarts the entire microcontroller
     */
    void restartMCU();

    /**
     * @brief Register Callback to trigger before update start
     *
     * @param cb Callback function, must return true for update to begin
     */
    void onBeforeUpdate(std::function<bool()>);

    /**
     * @brief Register Callback to trigger on update success
     *
     * @param cb Callback function, must take software version parameters
     */
    void onUpdateSuccess(std::function<void(String, String, String, String)> cb);
};
