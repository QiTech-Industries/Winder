#include <global.h>

class Webserver
{
private:
    struct request_s
    {
        String event;
        JsonObject data;
        String json;
    };

    struct queueJson_s
    {
        String event;
        std::function<String(JsonObject message)> cb;
    };

    struct queue_s
    {
        String event;
        std::function<String()> cb;
    };

    TimerForMethods<Webserver> dnsTimer;
    TimerForMethods<Webserver> socketTimer;
    AsyncWebServer *webServer = NULL;
    AsyncWebSocket *webSocket = NULL;
    DNSServer *dnsServer = NULL;
    request_s request;
    std::vector<queue_s> queue;
    std::vector<queueJson_s> queueJson;

    void dnsLoop()
    {
        dnsServer->processNextRequest();
    }

    void socketLoop()
    {
        webSocket->cleanupClients();
        // ping pong must be implemented to prevent too many messages in queue after network change
    }

    bool getMessage(void *arg, uint8_t *data, size_t len)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;

        if (info->opcode != WS_TEXT)
        {
            DEBUG_PRINTLN("Binary Messages are not supported.");
            return false;
        }
        if (info->index == 0)
        {
            request = {};
            DEBUG_PRINTLN("Socket received data.");
        }

        data[len] = 0;
        request.json += (char *)data;

        // return if first and only frame or last frame of multy frame message
        if (info->final && ((info->index == 0 && info->len == len) || ((info->index + len) == info->len)))
        {
            return true;
        }

        return true;
    }

    String constructMessage(String event, String data)
    {
        return String("{\"event\":\"") + event + String("\",\"data\":") + data + String("}");
    }

public:
    Webserver() : dnsTimer(this, &Webserver::dnsLoop), socketTimer(this, &Webserver::socketLoop)
    {
        dnsTimer.setInterval(10);
        socketTimer.setInterval(1000);
    }

    void create(uint16_t port, const char *defaultFile, const char *hostname, const char *friendly_name)
    {
        DEBUG_PRINTLN("[Server] Webserver started.");
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

        // create MDNS
        if (!MDNS.begin(hostname))
        {
            DEBUG_PRINTF("Error setting up MDNS %s port %u", hostname, port);
        }
        MDNS.setInstanceName(friendly_name);
        MDNS.addService("http", "tcp", port);
    }

    void createCaptive(IPAddress ip)
    {
        dnsServer = new DNSServer;
        dnsServer->setErrorReplyCode(DNSReplyCode::ServerFailure);
        dnsServer->start(53, "*", ip);
        dnsTimer.start();
    }

    void createSocket(String path)
    {
        webSocket = new AsyncWebSocket(path);
        webSocket->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                           {
                               String event;

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
                                   if (!getMessage(arg, data, len))
                                       return;

                                   StaticJsonDocument<512> doc;
                                   DeserializationError error = deserializeJson(doc, request.json);

                                   if (error)
                                   {
                                       DEBUG_PRINTLN("JSON could not be decoded.");
                                   }
                                   if (!doc["event"])
                                   {
                                       DEBUG_PRINTLN("JSON must contain event field.");
                                   }

                                   DEBUG_PRINTLN(request.json);
                                   request.event = doc["event"].as<String>();
                                   request.data = doc["data"];
                                   event = request.event;
                                   break;
                               }

                               //Check JSON queue for events
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

                               //Check queue for callbacks
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

    void on(String event, std::function<String()> cb)
    {
        queue.push_back({event, cb});
    }
    void on(String event, std::function<String(JsonObject message)> cb)
    {
        queueJson.push_back({event, cb});
    }
    void emit(String event, String data)
    {
        webSocket->textAll(constructMessage(event, data));
    }
};