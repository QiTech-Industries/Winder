#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LITTLEFS.h>
#include <logger/logging.h>
#include <timer.h>

const loggingLevel_e LOG_LEVEL = INFO;

// Required parameters for any input/output frame
struct frame_s {
    const char *event;  // Identifier of the event-type
    JsonObject data;    // Parsed data of request
};

// callback item
struct callback_s {
    const char *event;                           // Identifier of the event-type, required for triggering
    std::function<const char *(JsonObject)> cb;  // Callback-function, called when listener is triggered
};

/**
 * @brief Manages creation and handeling of dns, mdns and websockets
 */
class QiServer {
   private:
    TimerForMethods<QiServer> _dnsTimer;     // Timer for looping the dns-related code
    TimerForMethods<QiServer> _socketTimer;  // Timer for looping the socket-related code
    AsyncWebServer *_webServer = NULL;       // Handles incoming client-requests
    AsyncWebSocket *_webSocket = NULL;       // Handles incoming websockets of client
    DNSServer *_dnsServer = NULL;            // DNS server for handeling name resolution within the created access point
    std::vector<callback_s> _callbacks;      // vector containing all callbacks and corresponding events
    uint16_t _port = 80;                     // Port the webserver should start on
    char *_defaultFile = "index.html";       // File that should be served when accessing root domain

    /**
     * @brief Main loop for handeling the dns, called by respective timer
     */
    void handleDNS();

    /**
     * @brief Main loop for handeling the sockets, called by respective timer
     */
    void handleWS();

    /**
     * @brief Extract event param from JSON request
     *
     * @param data Pointer to request data
     * @param info Additional information about the request e.g. length
     * @return frame_s decoded JSON and event
     */
    frame_s decodeJsonRequest(uint8_t *data, AwsFrameInfo *info) {}

    /**
     * @brief Check callback vector for events with matching event-identifier
     *
     * @param event event to find callback for, usually extracted from request
     * @param data data to pass on to the callback
     * @return frame_s with response of callback
     */
    frame_s executeEventCallback(frame_s);

    /**
     * @brief act on incoming requests and extract their data
     *
     * @param type Websocket event provided by ESPAsyncWebserver
     * @param data Pointer to request data
     * @param info Additional information about the request e.g. length
     */
    frame_s parseRequest(AwsEventType type, uint8_t *data, AwsFrameInfo *info);

    /**
     * @brief Format event and data into standard response
     *
     * @param event identifier of the event-type
     * @param data message-data in a json string
     * @return String in standard response format {event: ..., data: ...}
     */
    String constructResponse(const char *event, JsonObject data);

   public:
    /**
     * @brief Constructor, immediately starts related timers
     */
    QiServer();

    /**
     * @brief Creates and starts the webserver at _port with _defaultFile
     *
     */
    void init();

    /**
     * @brief Creates and starts the webserver at specified port with specified defaultFile
     *
     * @param port port the webserver shall run on
     * @param defaultFile path of the default file relative to the LITTLEFS root-directory
     */
    void init(uint16_t port, char *defaultFile);

    /**
     * @brief Creates a MDNS Server to allow access at .local domain
     *
     * @param hostname hostname to make the webserver available under
     * @param friendly_name Name the device advertises itself under
     */
    void createMDNS(const char *hostname, const char *friendly_name);

    /**
     * @brief Creates and starts a dns-server for auto redirect to login page
     *
     * @param ip address of the dns-server
     */
    void createCaptive(IPAddress ip);

    /**
     * @brief Creates and starts the websocket handler at specified path
     *
     * @param path Path of resource, for example "/ws"
     */
    void createSocket(String path);

    /**
     * @brief Register a function to be called upon an event
     *
     * @param event identifier of the event-type
     * @param cb Callback function to be called when triggered
     */
    void on(const char *event, std::function<const char *(JsonObject)> cb);

    /**
     * @brief Sends a message to *all* connected clients
     *
     * @param event identifier of the event-type
     * @param data message-data in a json-compatible form (inserted into json unchecked)
     */
    void emit(const char *event, JsonObject data);

    /**
     * @brief Sends a message to *one* given client
     *
     * @param event identifier of the event-type
     * @param data message-data in a json-compatible form (inserted into json unchecked)
     */
    void emit(AsyncWebSocketClient *client, const char *event, JsonObject data);
};
