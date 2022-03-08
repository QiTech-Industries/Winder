#pragma once

// Related
// System / External
#include <Arduino.h> // Arduino standard library
#include <DNSServer.h> // Redirect every AP Request to Webinterface
#include <ArduinoJson.h> // Encode and Decode JSON messages 
#include <timer.h> // Timer to schedule Commands
#include <ESPAsyncWebServer.h> // Serve Website and create Websocket
// Selfmade
// Project

/**
 * @brief Manages creation and handeling of dns and websockets
 */
class Webserver{
    private:
        struct request_s // TODO - Comment
        {
            String event; // Identifier of the event-type
            JsonObject data; // TODO - Comment
            String json; // TODO - Comment, TODO: char* instead of String?
        };

        struct queueJson_s // Listener for json-messages of a specific event-type
        {
            String event; // Identifier of the event-type, required for triggering
            std::function<String(JsonObject message)> cb; // Callback-function, called when listener is triggered
        };

        struct queue_s // Listener for messages of a specific event-type
        {
            String event; // Identifier of the event-type, required for triggering
            std::function<String()> cb; // Callback-function, called when listener is triggered
        };

        TimerForMethods<Webserver> dnsTimer; // Timer for looping the dns-related code
        TimerForMethods<Webserver> socketTimer; // Timer for looping the socket-related code
        AsyncWebServer *webServer = NULL; // TODO - Comment
        AsyncWebSocket *webSocket = NULL; // TODO - Comment
        DNSServer *dnsServer = NULL; // DNS server for handeling name resolution within the created access point
        request_s request; // Buffer-variable for handeling incoming requests from the client // TODO: Check comment
        std::vector<queue_s> queue; // Listeners waiting to handle incoming client-messages // TODO: Rename to be more intuitive
        std::vector<queueJson_s> queueJson; // Listeners waiting to handle incoming client-messages that are formatted as json // TODO: Rename to be more intuitive
        
        /**
         * @brief Main loop for handeling the dns, called by respective timer
         */
        void dnsLoop();

        /**
         * @brief Main loop for handeling the sockets, called by respective timer
         */
        void socketLoop();

        /**
         * @brief Get next part of a message that is assumed to be in json-format and add it to the request.json (creating a new one for new messages) TODO: Check comment, also rename as it isn't a simple getter?
         * 
         * @param arg Pointer to the messageframe, that will be casted to AwsFrameInfo
         * @param data message-content
         * @param len length of message content
         * @return true data extracted
         * @return false error occurred (binary message)
         */
        bool getMessage(void *arg, uint8_t *data, size_t len);

        /**
         * @brief Combines the parameters into a message ready to be sent to the client via websocket
         * 
         * @param event identifier of the event-type, not checked for specia
         * @param data message-data in a json-compatible form (inserted into json unchecked)
         * @return String Message in the format "{event: <event>, data: <data>}"
         */
        String constructMessage(String event, String data);

    public:
        /**
         * @brief Constructor, immediately starts related timers
         */
        Webserver();

        /**
         * @brief Creates and starts the webserver, making it available over the network. TODO: Either rename to init() or move to constructor?
         * 
         * @param port port the webserver shall run on
         * @param defaultFile path of the default file relative to the LITTLEFS root-directory
         * @param hostname hostname to make the webserver available under
         * @param friendly_name TODO: Comment
         */
        void create(uint16_t port, const char *defaultFile, const char *hostname, const char *friendly_name);

        /**
         * @brief Creates and starts a dns-server
         * 
         * @param ip adress of the dns-server
         */
        void createCaptive(IPAddress ip);

        /**
         * @brief Creates and starts the websocket handler, maybe rename to startWebSocket() without parameters, since only one will be available? Otherwise the Handler should also be added as a parameter
         * 
         * @param path Path of resource, for example "/ws"
         */
        void createSocket(String path);

        /**
         * @brief Register a function to be called upon a an event
         * 
         * @param event identifier of the event-type
         * @param cb Callback function to be called when triggered
         */
        void on(String event, std::function<String()> cb);

        /**
         * @brief Register a function to be called upon a an event
         * 
         * @param event identifier of the event-type
         * @param cb Callback function to be called when triggered
         */
        void on(String event, std::function<String(JsonObject message)> cb);

        /**
         * @brief Sends a message to the client via websocket
         * 
         * @param event identifier of the event-type
         * @param data message-data in a json-compatible form (inserted into json unchecked)
         */
        void emit(String event, String data);
};