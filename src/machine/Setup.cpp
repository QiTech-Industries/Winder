#include <machine/Setup.h>

// TODO once clang 14 is supported use AlignAfterOpenBracket: BlockIndent option
void Winder::printBanner() {
    Serial.print(
        "\n   ___                  _       _    _ _           _           "
        "\n  |_  |                (_)     | |  | (_)         | |          "
        "\n    | | __ _ _ ____   ___ ___  | |  | |_ _ __   __| | ___ _ __ "
        "\n    | |/ _` | '__\\ \\ / / / __| | |/\\| | | '_ \\ / _` |/ _ \\ '__|"
        "\n/\\__/ / (_| | |   \\ V /| \\__ \\ \\  /\\  / | | | | (_| |  __/ |   "
        "\n\\____/ \\__,_|_|    \\_/ |_|___/  \\/  \\/|_|_| |_|\\__,_|\\___|_|   \n");
}

Winder::Winder() : _updater(*this), _wifi(*this) { _wifi.start(); }

winderSoftwareConfiguration_s& Winder::getSoftConfiguration() { return _softC; }

float parseMpmToRpm(JsonObject data, float mmPerRotation) {
    // TODO: move 14 to hard config
    const float MAX_SPEED_MPM = 14;  // spool load recognition is not possible at faster speeds
    return mpmToRpm(putNumberInRange(data["mpm"].as<float>(), 0, MAX_SPEED_MPM), mmPerRotation);
}

if (!WiFi.isConnected() || Update.isRunning() ||
    (_machine.getCurrentMode() != OPERATE_OFF && _machine.getCurrentMode() != OPERATE_STANDBY)) {
    return;
}

void QiUpdater::restartMCU() {
    ESP.restart();
    while (true) {
    };  // Wait for microcontroller to finally die
}

strcpy(getConfiguration().software.spiffs.version, XVersion.c_str());
strcpy(getConfiguration().software.spiffs.build, XBuild.c_str());
strcpy(getConfiguration().software.spiffs.date, date.substring(0, 16).c_str());

strcpy(getConfiguration().software.firmware.version, XVersion.c_str());
strcpy(getConfiguration().software.firmware.build, XBuild.c_str());
strcpy(getConfiguration().software.firmware.date, date.substring(0, 16).c_str());


void Winder::init(winderConfiguration_s conf) {
    QiWifi::init();
    QiUpdater::init();
    QiWebserver::init();

    printBanner();

    // Save and read configuration
    _softC = conf.soft;
    _hardC = conf.hard;

    if (!conf.isValid()) return;

    // Initialize stepper motors
    _engine = new FastAccelStepperEngine();
    _stepperSpool = new Stepper(_hardC.motors.spool, _engine);
    _stepperFerrari = new Stepper(_hardC.motors.ferrari, _engine);
    _stepperPuller = new Stepper(_hardC.motors.puller, _engine);
    _engine->init();
    _stepperSpool->init();
    _stepperFerrari->init();
    _stepperPuller->init();
    _stepperSpool->setDebuggingLevel(ERROR);
    _stepperFerrari->setDebuggingLevel(ERROR);
    _stepperPuller->setDebuggingLevel(ERROR);

    // Load previous configuration from file system
    Serial.print("[Store] Default config: ");
    Serial.println(_softC.asJSON());
    if (!_softC.load()) {
        // Failed to load configuration, return to specified values
        _softC = conf.soft;
    }
    Serial.print("[Store] Using config: ");
    Serial.println(_softC.asJSON());

    // Create HTTP server and websocket
    _server.create(_hardC.server.port, _hardC.server.default_path, _softC.wifi.mdns_name, _softC.wifi.friendly_name);
    _server.createSocket("/ws");

    // Connect to Wifi and create AP
    if (strlen(_softC.wifi.ssid)) {
        _wifi.connect(_softC.wifi.ssid, _softC.wifi.password);
    }
    if (_softC.wifi.ap_enabled || !strlen(_softC.wifi.ssid)) {
        _wifi.createAP(_softC.wifi.ap_ssid, _softC.wifi.ap_password);
        _server.createCaptive(WiFi.softAPIP());
    }

    // Add listener for Wifi connection changes
    _wifi.setConnectionChangeCallback([=]() {
        switch (_wifi.getConnectionMode()) {
            case OFFLINE:
                _server.emit("connect", "\"failed\"");
                break;
            case ONLINE:
                _server.emit("connect", "\"connected\"");
                break;
            default:
                break;
        }
    });

    // Start Timers/Threads
    _updater.setInterval(30000);
    _timedReporter = new TimerForMethods<Winder>(this, &Winder::handleStatusReport);
    _timedSpeedAdjuster = new TimerForMethods<Winder>(this, &Winder::handleSpeedAdjust);
    _timedReporter->setInterval(1000);
    _timedReporter->start();
    _timedSpeedAdjuster->setInterval(100);
    _timedSpeedAdjuster->start();

    // Start in Standby by default
    operateStandby();
}

void Winder::handle() {
    // TODO: activate/deactivate with environment variable
    // Interactive key-input based debugging
    /*
    if(Serial.available() > 0){
        uint8_t newCommand = Serial.read();
        JarvisWinder.executeDebugCommand(newCommand);
    }
    */

    QiWifi::handle();
    QiUpdater::handle();
    QiWebserver::handle();
    TimerManager::instance().update();

    _stepperSpool->handle();
    _stepperFerrari->handle();
    _stepperPuller->handle();
}

void Winder::adjustSpeed(float speedMetersPerMinute) {
    logPrint(INFO, INFO, "Winder::adjustSpeed speedMpm: %.2f\n", speedMetersPerMinute);

    // Adjust current operation if any
    switch (_currentWinderOperation) {
        case OPERATE_OFF:
        case OPERATE_STANDBY:
            return;
        case OPERATE_PULLING:
            operatePull(speedMetersPerMinute);
            break;
        case OPERATE_WINDING:
            operateWind(speedMetersPerMinute);
            break;
        case OPERATE_UNWINDING:
            operateUnwind(speedMetersPerMinute);
            break;
        case OPERATE_CALIBRATING:
            // Not implemented as separate mode
            break;
    }
}

void Winder::handleStatusReport() {
    if (_wifi.getConnectionMode() == CONNECTING) return;

    stepperStatus_s spoolStatus = _stepperSpool->getStatus();
    stepperStatus_s ferrariStatus = _stepperFerrari->getStatus();
    stepperStatus_s pullerStatus = _stepperPuller->getStatus();

    // Assemble report message
    StaticJsonDocument<512> doc;
    String json;
    doc["f"]["r"] = ferrariStatus.rpm;                            // Ferrari-rpm
    doc["f"]["s"] = ferrariStatus.load;                           // Ferrari-stall (load)
    doc["f"]["a"] = _stepperFerrari->getCurrentMode() != OFF;     // Ferrari-active
    doc["p"]["r"] = pullerStatus.rpm;                             // Puller-rpm
    doc["p"]["s"] = pullerStatus.load;                            // Puller-stall (load)
    doc["p"]["a"] = _stepperPuller->getCurrentMode() != OFF;      // Puller-active
    doc["s"]["r"] = spoolStatus.rpm;                              // Spool-rpm
    doc["s"]["s"] = spoolStatus.load;                             // Spool-stall (load)
    doc["s"]["a"] = _stepperSpool->getCurrentMode() != OFF;       // Spool-active
    doc["m"] = winderOperationToString(_currentWinderOperation);  // Current machine-operation
    doc["e"] = nullptr;                                           // Errors // TODO: Re-implement
    doc["w"] = 0;                                                 // Total windings // TODO spoolStatus.rotations;
    doc["l"] = 0;  // Total wound length // TODO ferrariStatus.rotations * hard.motors.ferrari.mm_per_rotation / 1000;

    // Serialise and check message
    if (serializeJson(doc, json) == 0) {
        Serial.println("[Winder] JSON serialization failed!");  // TODO: Debug/Logging
        // TODO: return if json failed?
    }

    // Send message
    _server.emit("stats", json);
}

float Winder::calculateSpoolToFerrariSpeedRatio() {
    const float SPOOL_GEAR_RATIO = 1;  // 5.18; // TODO - Comment - hard.motors.spool.gearRatio
    const float FILAMENT_SIZE = 1.75;  // Width of filament in mm

    float spoolWidthMm = _softC.ferrari_max - _softC.ferrari_min;  // Width of spool in mm
    float spoolMotorRotationsPerLayer =
        spoolWidthMm * SPOOL_GEAR_RATIO / FILAMENT_SIZE;  // Number of spool-rotations needed for winding an entire layer of filament
    float ferrariMotorRotationsPerLayer =
        spoolWidthMm / _hardC.motors.ferrari.mmPerRotation;  // Number of ferrari-rotations needed for winding an entire layer of filament
    float speedRatio = spoolMotorRotationsPerLayer / ferrariMotorRotationsPerLayer;

    return speedRatio;
}

void Winder::handleSpeedAdjust() {
    if (_stepperFerrari->getCurrentMode() == OSCILLATING_FORWARD || _stepperFerrari->getCurrentMode() == OSCILLATING_BACKWARD) {
        float speedNewRpm = _stepperSpool->getStatus().rpm / calculateSpoolToFerrariSpeedRation();
        _stepperFerrari->adjustMoveSpeed(_stepperFerrari->getStatus().rpm < 0 ? min(-speedNewRpm, speedNewRpm)
                                                                              : max(-speedNewRpm, speedNewRpm));
    }
}

uint32_t speedRpmToMpm(float speedRpm, float mmPerRotation) { return speedRpm / 1000 * mmPerRotation; }

winderOperation_e Winder::getCurrentMode() { return _currentWinderOperation; }
