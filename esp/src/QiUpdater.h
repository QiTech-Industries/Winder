#pragma once

// Related
// System / External
#include <string>
#include <stdint.h>
#include <timer.h>
#include <HTTPClient.h>
// Selfmade
// Project
#include <QiMachineWinderConfiguration.h>

class QiMachineWinder;

/**
 * @brief Checks for updates frequently and downloads/installs them when available
 */
class QiUpdater
{
private:
    QiMachineWinder& _machine; // Reference to the machine we manage the wifi for, needed for requesting the configuration
    const String overTheAirURL = "http://update.qitech.de/"; // Url to check for updates // TODO: String and potentially setting externally (setup struct or in hard configuration?)
    const char *headerKeys[4] = {"X-Update", "X-Build", "X-Version", "date"}; // HTTP-Headers needed for extract the headers, needs to be an array, since it is used as a parameter for the http-client
    TimerForMethods<QiUpdater> timer; // Timer for main looping of handeling
    HTTPClient http; // Client for requesting data over the network

    /**
     * @brief Get software configuration of machine, TODO: Helper function for transition away from global variables
     */
    configurationMachineWinderSoftware_s& getConfiguration();

public:
    /**
     * @brief Constructor
     * 
     * @param winder Reference to machine to work with
     */
    QiUpdater(QiMachineWinder& winder);

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
     * @brief Repeatedly called by timer, checks for new updates, downloads / installs them when available
     */
    void handle();
};
