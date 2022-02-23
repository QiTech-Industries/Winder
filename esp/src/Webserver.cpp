#include <Webserver.h>

// TODO: Replace with proper debugging/logging
#ifdef DEBUG_WINDER
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x...)
#endif

// Built in Library inclusions
#include <Arduino.h> // Arduino standard library
#include <DNSServer.h> // Redirect every AP Request to Webinterface
#include <ESPmDNS.h> // Make Interface available at winder.local

// External Library inclusions
#include <ArduinoJson.h> // Encode and Decode JSON messages 
#include <timer.h> // Timer to schedule Commands
#include <ESPAsyncWebServer.h> // Serve Website and create Websocket
#include <LITTLEFS.h> // Store Webinterface in Filesystem

void Webserver::dnsLoop()
{
    dnsServer->processNextRequest();
}

void Webserver::socketLoop()
{
    webSocket->cleanupClients();
    // ping pong must be implemented to prevent too many messages in queue after network change
}

bool Webserver::getMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;

    if (info->opcode != WS_TEXT)
    {
        DEBUG_PRINTLN("Binary Messages are not supported."); // TODO: Debugging / Logging
        return false;
    }
    if (info->index == 0)
        request = {};

    data[len] = 0;
    request.json += (char *)data;

    // return if first and only frame or last frame of multi frame message
    // TODO: What is the point of that check if the return value will be true either way?
    if (info->final && ((info->index == 0 && info->len == len) || ((info->index + len) == info->len)))
    {
        return true;
    }

    return true;
}

String Webserver::constructMessage(String event, String data)
{
    return String("{\"event\":\"") + event + String("\",\"data\":") + data + String("}");
}

Webserver::Webserver() : dnsTimer(this, &Webserver::dnsLoop), socketTimer(this, &Webserver::socketLoop)
{
    dnsTimer.setInterval(10);
    socketTimer.setInterval(1000);
}

void Webserver::create(uint16_t port, const char *defaultFile, const char *hostname, const char *friendly_name)
{
    DEBUG_PRINTLN("[Server] Webserver started."); // TODO: Debug/Logging
    webServer = new AsyncWebServer(port);
    webServer->serveStatic("/", LITTLEFS, "/")
        .setDefaultFile(defaultFile)
        .setCacheControl("max-age=600")
        .setFilter([hostname](AsyncWebServerRequest *request)
                    { return request->host() == String(hostname) + ".local" || WiFi.localIP() == request->client()->localIP(); });
    // AP mode: redirect to winder.local on any domain name or winder ip
    // STA mode: allow winder IP and winder.local as Android does not support mDNS
    webServer->onNotFound([hostname](AsyncWebServerRequest *request)
                            { request->redirect(String("http://") + hostname + ".local"); });

    webServer->begin();
    DEBUG_PRINTLN(String("[Server] Available under http://") + hostname + ".local"); // TODO: Debug/Logging

    // create MDNS
    if (!MDNS.begin(hostname))
    {
        DEBUG_PRINTF("[Server] Error setting up MDNS %s port %u", hostname, port);
    }
    MDNS.setInstanceName(friendly_name);
    MDNS.addService("http", "tcp", port);
}

void Webserver::createCaptive(IPAddress ip)
{
    dnsServer = new DNSServer;
    dnsServer->setErrorReplyCode(DNSReplyCode::ServerFailure);
    dnsServer->start(53, "*", ip);
    dnsTimer.start();
}

void Webserver::createSocket(String path)
{
    DEBUG_PRINTLN("[Server] Websocket created");
    webSocket = new AsyncWebSocket(path);
    webSocket->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                        {
                            String event;
                            
                            // Determine type of incoming message and set internal event-identifier accordingly for further handeling
                            switch (type)
                            {
                            case WS_EVT_CONNECT:
                                event = "ws_connect";
                                DEBUG_PRINTLN("[Server] Client connected to Websocket.");
                                break;
                            case WS_EVT_DISCONNECT:
                                event = "ws_disconnect";
                                DEBUG_PRINTLN("[Server] Client disconnected from Websocket.");
                                break;
                            case WS_EVT_ERROR:
                                event = "error";
                                break;
                            case WS_EVT_PONG:
                                return;
                                break;
                            case WS_EVT_DATA:
                                // Date received, try to read json and read event-identifier from message
                                if (!getMessage(arg, data, len))
                                    return;

                                // Read and check json for validity
                                StaticJsonDocument<512> doc;
                                DeserializationError error = deserializeJson(doc, request.json);
                                if (error) {
                                    DEBUG_PRINTLN("[Server] JSON could not be decoded.");
                                }
                                if (!doc["event"]) {
                                    DEBUG_PRINTLN("[Server] JSON must contain event field.");
                                }
                                // TODO: abort on error?
                                
                                // Extract data from deserialised json
                                DEBUG_PRINTLN("[Server]" + request.json);
                                request.event = doc["event"].as<String>();
                                request.data = doc["data"];
                                event = request.event;
                                break;
                            }

                            //Check JSON queue for events with matching event-identifier
                            // Send the json-converted answer of the called function back to the client via websocket
                            for (auto i = queueJson.begin(); i != queueJson.end(); i++)
                            {
                                if (i->event != event || !i->cb)
                                    continue;

                                String data = i->cb(request.data);
                                if (data == __null)
                                    return;
                                client->text(constructMessage(event, data));
                                return;
                            }

                            // Check queue for callbacks with matching event-identifier
                            // Send the answer of the called function back to the client via websocket
                            for (auto i = queue.begin(); i != queue.end(); i++)
                            {
                                if (i->event != event || !i->cb)
                                    continue;

                                String data = i->cb();
                                if (data == __null)
                                    return;
                                client->text(data);
                            }
                        });
    webServer->addHandler(webSocket);
    socketTimer.start();
}

void Webserver::on(String event, std::function<String()> cb)
{
    queue.push_back({event, cb});
}

void Webserver::on(String event, std::function<String(JsonObject message)> cb)
{
    queueJson.push_back({event, cb});
}

void Webserver::emit(String event, String data)
{
    webSocket->textAll(constructMessage(event, data));
}