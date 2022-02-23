#pragma once

#include <string>
#include <stdint.h>
#include <timer.h>
#include <HTTPClient.h>

/**
 * @brief Checks for updates frequently and downloads/installs them when available
 */
class QiUpdater
{
private:
    String overTheAirURL = "http://update.qitech.de/"; // Url to check for updates // TODO: String and potentially setting externally (setup struct?)
    const char *headerKeys[4] = {"X-Update", "X-Build", "X-Version", "date"}; // HTTP-Headers needed for extract the headers, needs to be an array, since it is used as a parameter for the http-client
    TimerForMethods<QiUpdater> timer; // Timer for main looping of handeling
    HTTPClient http; // Client for requesting data over the network

public:
    // Constructor
    QiUpdater();

    /**
     * @brief Restarts the entire microcontroller
     */
    void restartMCU();

    /**
     * Sets and starts the interval of the internal timer
     * 
     * @param interval interval in ms
     */
    void setInterval(uint16_t interval);

    /**
     * @brief Repeatedly called by timer, checks for new updates, downloads / installs them when available and triggers restarts of the microcontroller as needed
     */
    void check(); // TODO: Rename to handle() ?
};