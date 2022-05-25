#include <machine/QiServer.h>

void QiServer::handleDNS() { _dnsServer->processNextRequest(); }

void QiServer::handleWS() {
    _webSocket->cleanupClients();
    // ping pong must be implemented to prevent too many messages in queue after network change
    // error: Too many messages queued
}

frame_s QiServer::decodeJsonRequest(uint8_t *data, AwsFrameInfo *info) {
    // only text messages are supported
    if (info->opcode != WS_TEXT) return frame_s{"ws_binary"};

    // One Frame can be up to 1MB, multi frame messages must be assembled manually and are currently not supported
    if (!info->final) return frame_s{"ws_multiframe"};

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, data);

    // In case the json data can't be decoded or does not have correct format
    if (error || !doc["event"] || !doc["data"]) return frame_s{"ws_invalid"};

    return frame_s{doc["event"].as<const char *>(), doc["data"].as<JsonObject>()};
}

frame_s QiServer::parseRequest(AwsEventType type, uint8_t *data, AwsFrameInfo *info) {
    frame_s request;

    // Determine type of incoming message and set internal event-identifier accordingly for further handeling
    // Internal Websocket events are prefixed with ws_
    switch (type) {
        case WS_EVT_CONNECT:
            request.event = "ws_connect";
            break;
        case WS_EVT_DISCONNECT:
            request.event = "ws_disconnect";
            break;
        case WS_EVT_ERROR:
            request.event = "ws_error";
            break;
        case WS_EVT_PONG:
            request.event = "ws_pong";
            break;
        case WS_EVT_DATA: {
            request = decodeJsonRequest(data, info);
            break;
        }
        default:
            request.event = "ws_unknown";
            break;
    }

    logPrint(LOG_LEVEL, INFO, "[Server] %s Event received\n", request.event);
    return request;
}

String QiServer::constructResponse(const char *event, JsonObject data) {
    StaticJsonDocument<512> doc;
    doc["data"] = data;
    doc["event"] = event;
    return doc.as<String>();
}

QiServer::QiServer() : _dnsTimer(this, &QiServer::handleDNS), _socketTimer(this, &QiServer::handleWS) {
    _dnsTimer.setInterval(10);
    _socketTimer.setInterval(1000);
}

void QiServer::init() {
    logPrint(LOG_LEVEL, INFO, "[Server] Webserver started on Port %u\n", _port);

    _webServer = new AsyncWebServer(_port);
    _webServer->serveStatic("/", LITTLEFS, "/").setDefaultFile(_defaultFile).setCacheControl("max-age=600");

    // Redirect to Web Root if path not found instead of 404 error
    _webServer->onNotFound([](AsyncWebServerRequest *request) { request->redirect("/"); });
    _webServer->begin();
}

void QiServer::init(uint16_t port, char *defaultFile) {
    _port = port;
    _defaultFile = defaultFile;

    init();
}

void QiServer::createMDNS(const char *hostname, const char *friendly_name) {
    if (!MDNS.begin(hostname)) {
        logPrint(LOG_LEVEL, ERROR, "[Server] Error setting up MDNS at http://%s.local\n", hostname);
        return;
    }

    MDNS.setInstanceName(friendly_name);
    MDNS.addService("http", "tcp", _port);
    logPrint(LOG_LEVEL, INFO, "[Server] Available at http://%s.local\n", hostname);
}

void QiServer::createCaptive(IPAddress ip) {
    _dnsServer = new DNSServer;
    _dnsServer->setErrorReplyCode(DNSReplyCode::ServerFailure);

    if (_dnsServer->start(53, "*", ip)) {
        _dnsTimer.start();
        logPrint(LOG_LEVEL, INFO, "[Server] Captive Portal successfully created");
        return;
    }

    logPrint(LOG_LEVEL, ERROR, "[Server] No Sockets for Captive Portal available\n");
}

frame_s QiServer::executeEventCallback(frame_s request) {
    for (const auto &callback : _callbacks) {
        if (callback.event != request.event || !callback.cb) continue;

        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, callback.cb(request.data));
        if (error) logPrint(LOG_LEVEL, ERROR, "[Server] Invalid JSON returned from WS Callback\n");
        return frame_s{request.event, doc.as<JsonObject>()};
    }

    logPrint(LOG_LEVEL, WARNING, "[Server] No matching callback for event found\n");
    return frame_s{};
}

void QiServer::createSocket(String path) {
    logPrint(LOG_LEVEL, INFO, "[Server] Websocket created\n");

    _webSocket = new AsyncWebSocket(path);
    _webSocket->onEvent([=](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        frame_s request = parseRequest(type, data, info);
        frame_s response = executeEventCallback(request);
        if (response.event) emit(client, response.event, response.data);
    });
    _webServer->addHandler(_webSocket);
    _socketTimer.start();
}

void QiServer::on(const char *event, std::function<const char *(JsonObject)> cb) { _callbacks.push_back({event, cb}); }

void QiServer::emit(AsyncWebSocketClient *client, const char *event, JsonObject data) { client->text(constructResponse(event, data)); }

void QiServer::emit(const char *event, JsonObject data) { _webSocket->textAll(constructResponse(event, data)); }
