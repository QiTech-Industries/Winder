// Related
#include <QiMachineWinder.h>
// System / External
#include <timer.h>
#include <timerManager.h>
// Selfmade
#include "controller/stepper/Stepper.h"
// Project
#include <QiMachineWinderConfiguration.h>

void QiMachineWinder::printBanner() {
    Serial.print(
        "\n   ___                  _       _    _ _           _           "
        "\n  |_  |                (_)     | |  | (_)         | |          "
        "\n    | | __ _ _ ____   ___ ___  | |  | |_ _ __   __| | ___ _ __ "
        "\n    | |/ _` | '__\\ \\ / / / __| | |/\\| | | '_ \\ / _` |/ _ \\ '__|"
        "\n/\\__/ / (_| | |   \\ V /| \\__ \\ \\  /\\  / | | | | (_| |  __/ |   "
        "\n\\____/ \\__,_|_|    \\_/ |_|___/  \\/  \\/|_|_| |_|\\__,_|\\___|_|   \n");
}

QiMachineWinder::QiMachineWinder() : _updater(*this), _wifi(*this) { _wifi.start(); }

configurationMachineWinderSoftware_s& QiMachineWinder::getConfigurationSoft() { return _configurationSoft; }

float QiMachineWinder::fixSpeedValue(float speedMetersPerMinute) {
    if (speedMetersPerMinute < 0) return 0;
    if (MAX_SPEED_MPM > 0 && speedMetersPerMinute > MAX_SPEED_MPM) return MAX_SPEED_MPM;
    return speedMetersPerMinute;
}

void QiMachineWinder::init(configurationMachineWinder_s conf) {
    printBanner();

    // Save and read configuration
    _configurationSoft = conf.soft;
    _configurationHard = conf.hard;

    // Check pin-configuration
    McValidatorEsp32 pinValidator;
    uint8_t pins[] = {
        _configurationHard.motors.spool.pins.cs,   _configurationHard.motors.spool.pins.dir,    _configurationHard.motors.spool.pins.en,
        _configurationHard.motors.spool.pins.step, _configurationHard.motors.ferrari.pins.cs,   _configurationHard.motors.ferrari.pins.dir,
        _configurationHard.motors.ferrari.pins.en, _configurationHard.motors.ferrari.pins.step, _configurationHard.motors.puller.pins.cs,
        _configurationHard.motors.puller.pins.dir, _configurationHard.motors.puller.pins.en,    _configurationHard.motors.puller.pins.step};
    if (!pinValidator.isDigitalPin(pins, 12)) return;

    // Initialize stepper motors
    _engine = new FastAccelStepperEngine();
    _stepperSpool = new Stepper(_configurationHard.motors.spool, _engine);
    _stepperFerrari = new Stepper(_configurationHard.motors.ferrari, _engine);
    _stepperPuller = new Stepper(_configurationHard.motors.puller, _engine);
    _engine->init();
    _stepperSpool->init();
    _stepperFerrari->init();
    _stepperPuller->init();
    _stepperFerrari->setHomingSpeed(120);
    _stepperSpool->setDebuggingLevel(ERROR);
    _stepperFerrari->setDebuggingLevel(ERROR);
    _stepperPuller->setDebuggingLevel(ERROR);

    // Load previous configuration from file system
    Serial.print("[Store] Default config: ");
    Serial.println(_configurationSoft.asJSON());
    if (!_configurationSoft.load()) {
        // Failed to load configuration, return to specified values
        _configurationSoft = conf.soft;
    }
    Serial.print("[Store] Using config: ");
    Serial.println(_configurationSoft.asJSON());

    // Create HTTP server and websocket
    _server.create(_configurationHard.server.port, _configurationHard.server.default_path, _configurationSoft.wifi.mdns_name,
                   _configurationSoft.wifi.friendly_name);
    _server.createSocket("/ws");

    // Add listeners for incoming socket events
    _server.on("connect", [=](JsonObject data) {  // Connect to a wifi with a given ssid and password
        _wifi.connect(data["name"], data["password"]);
        return String();
    });
    _server.on("scan", [=](JsonObject data) {  // Return list of available wifis
        return _wifi.scan();
    });
    _server.on("power", [=](JsonObject data) {  // Put machine into active mode, powering on the motors
        operateStandby();
        return String();
    });
    _server.on("standby", [=](JsonObject data) {  // Put machine into standby, powering off the motors
        operateOff();
        return String();
    });
    _server.on("calibrate", [=](JsonObject data) {  // Set new start- or end-position for the ferrari-oscillation
        // Parse parameters (no validity check necessary)
        uint16_t newPositionValue = data["position"].as<uint16_t>();
        bool newPositionIsStart = data["startPos"].as<bool>();

        // Execute command
        if (_currentWinderOperation == OPERATE_STANDBY || _currentWinderOperation == OPERATE_OFF) {
            // Currently idleing, lets immediately move to the new position to make calibration easier
            operateCalibrate(newPositionValue);
            if (newPositionIsStart) {
                _configurationSoft.ferrari_min = newPositionValue;
            } else {
                _configurationSoft.ferrari_max = newPositionValue;
            }
        } else {
            // Currently moving, adjust positions smoothly to avoid interruption of ferrari-movement
            if (newPositionIsStart) {
                adjustOscillationPositions(_configurationSoft.ferrari_max, newPositionValue);
            } else {
                adjustOscillationPositions(newPositionValue, _configurationSoft.ferrari_min);
            }
        }

        // Store new configuration
        _configurationSoft.store();  // TODO: must be called async or esp might crash on watchdog trigger
        return String();
    });
    _server.on("wind", [=](JsonObject data) {  // Start winding at a defined speed
        // Parse parameter
        float mpm = data["mpm"].as<float>();  // Speed in meters per minute (based on diameter of the puller-role)
        mpm = fixSpeedValue(mpm);

        // Execute command
        operateWind(mpm);
        return String();
    });
    _server.on("unwind", [=](JsonObject data) {  // Start unwinding at a defined speed
        // Parse parameter
        float mpm = data["mpm"].as<float>();  // Speed in meters per minute (based on diameter of the puller-role)
        mpm = fixSpeedValue(mpm);

        // Execute command
        operateUnwind(mpm);
        return String();
    });
    _server.on("pull", [=](JsonObject data) {  // Start pulling at a defined speed
        // Parse and check parameter
        float mpm = data["mpm"].as<float>();  // Speed in meters per minute (based on diameter of the puller-role)
        mpm = fixSpeedValue(mpm);

        // Execute command
        operatePull(mpm);
        return String();
    });
    _server.on("change", [=](JsonObject data) {  // Change spool - same as pulling
        // TODO: "change" is a really unintuitive command-identifier

        // Execute command
        operatePull(
            speedRpmToMpm(_stepperPuller->getStatus().rpm,
                          _configurationHard.motors.puller.mmPerRotation));  // Keep pulling with same speed since we are already winding
        return String();
    });
    _server.on("speed", [=](JsonObject data) {  // Adjust speed while pulling/unwinding/winding
        // Parse and check parameter
        float mpm = data["mpm"].as<float>();  // Speed in meters per minute (based on diameter of the puller-role)
        mpm = fixSpeedValue(mpm);

        // Execute command
        adjustSpeed(mpm);
        return String();
    });
    _server.on("modify", [=](JsonObject data) {  // Change the wifi-configuration
        // TODO: "modify" is a really unintuitive command-identifier
        _configurationSoft.wifi.ap_enabled = data["ap_enabled"];
        if (data["mdns_name"] != "") {
            strcpy(_configurationSoft.wifi.mdns_name, data["mdns_name"]);
        }
        if (data["ap_ssid"] != "") {
            strcpy(_configurationSoft.wifi.ap_ssid, data["ap_ssid"]);
        }

        // Apply wifi-related changes on the fly
        // TODO: Implement

        // Store new configuration
        _configurationSoft.store();  // TODO: must be called async or esp might crash on watchdog trigger

        // TODO: Manually force configuration save?
        return String("\"stored\"");
    });
    _server.on("config", [=](JsonObject data) {  // Read current configuration
        return _configurationSoft.asJSON();
    });

    // Connect to Wifi and create AP
    if (strlen(_configurationSoft.wifi.ssid)) {
        _wifi.connect(_configurationSoft.wifi.ssid, _configurationSoft.wifi.password);
    }
    if (_configurationSoft.wifi.ap_enabled || !strlen(_configurationSoft.wifi.ssid)) {
        _wifi.createAP(_configurationSoft.wifi.ap_ssid, _configurationSoft.wifi.ap_password);
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
    _timedReporter = new TimerForMethods<QiMachineWinder>(this, &QiMachineWinder::handleStatusReport);
    _timedSpeedAdjuster = new TimerForMethods<QiMachineWinder>(this, &QiMachineWinder::handleSpeedAdjust);
    _timedReporter->setInterval(1000);
    _timedReporter->start();
    _timedSpeedAdjuster->setInterval(100);
    _timedSpeedAdjuster->start();

    // Start in Standby by default
    operateStandby();
}

void QiMachineWinder::handle() {
    TimerManager::instance().update();

    _stepperSpool->handle();
    _stepperFerrari->handle();
    _stepperPuller->handle();
}

void QiMachineWinder::operatePull(float speedMetersPerMinute) {
    float speedRpm = speedMetersPerMinute * 1000 / _configurationHard.motors.puller.mmPerRotation;
    logPrint(INFO, INFO, "QiMachineWinder::operatePull speedMpm: %.2f, speedRpm: %.2f, oscillation: {start: %d, end: %d, offset: %d}\n",
             speedMetersPerMinute, speedRpm, _configurationSoft.ferrari_min, _configurationSoft.ferrari_max,
             _configurationHard.motors.ferrariOffset);  // TODO: DEBUG

    // Rotate puller, disable spool, position ferrari at start position for following winding
    _stepperSpool->switchModeOff();
    _stepperFerrari->movePosition(-FERRARI_POSITIONING_SPEED, _configurationSoft.ferrari_max - _configurationHard.motors.ferrariOffset);
    if (_currentWinderOperation == OPERATE_PULLING || _currentWinderOperation == OPERATE_WINDING) {
        // Keep rotating without interrupt
        _stepperPuller->adjustMoveSpeed(-speedRpm);
    } else {
        _stepperPuller->moveRotate(-speedRpm);
    }
    _currentWinderOperation = OPERATE_PULLING;
}

void QiMachineWinder::operateUnwind(float speedMetersPerMinute) {
    logPrint(INFO, INFO, "QiMachineWinder::operateUnwind\n");  // TODO: DEBUG
    float speedRpm = speedMetersPerMinute * 1000 / _configurationHard.motors.puller.mmPerRotation;

    _stepperSpool->switchModeOff();
    _stepperFerrari->movePosition(-FERRARI_POSITIONING_SPEED, _configurationSoft.ferrari_max - _configurationHard.motors.ferrariOffset);
    _stepperPuller->moveRotate(speedRpm);
    _currentWinderOperation = OPERATE_UNWINDING;
}

void QiMachineWinder::operateWind(float speedMetersPerMinute) {
    float speedRpm = speedMetersPerMinute * 1000 / _configurationHard.motors.puller.mmPerRotation;
    uint8_t DESIRED_SPOOL_LOAD = 50;  // Desired spool load in percent (0-100)
    logPrint(INFO, INFO, "QiMachineWinder::operateWind, start: %d, end: %d, speedRpmPuller: %.2f, desiredLoad: %d\n",
             _configurationSoft.ferrari_min, _configurationSoft.ferrari_max, speedRpm, DESIRED_SPOOL_LOAD);  // TODO: DEBUG

    if (_currentWinderOperation == OPERATE_PULLING || _currentWinderOperation == OPERATE_WINDING) {
        // Keep rotating without interrupt
        _stepperPuller->adjustMoveSpeed(-speedRpm);
        if (_currentWinderOperation == OPERATE_WINDING) {
            _stepperSpool->adjustMoveSpeed(-speedRpm / 5);  // TODO: Hardcoded
        }
    } else {
        _stepperPuller->moveRotate(-speedRpm);
    }
    if (_currentWinderOperation != OPERATE_WINDING) {
        _stepperSpool->moveRotateWithLoadAdjust(-speedRpm / 5, 15);  // TODO: Hardcoded - DESIRED_SPOOL_LOAD?
        _stepperFerrari->moveOscillate(
            speedRpm, _configurationSoft.ferrari_max - _configurationHard.motors.ferrariOffset,
            _configurationSoft.ferrari_min -
                _configurationHard.motors.ferrariOffset);  // Oscillate, speed will be adjusted repeatedly in handleSpeedAdjust()
    }

    _currentWinderOperation = OPERATE_WINDING;
}

void QiMachineWinder::operateCalibrate(uint16_t calibrationPosition) {
    logPrint(INFO, INFO, "QiMachineWinder::operateCalibrate\n");  // TODO: DEBUG
    _stepperSpool->switchModeStandby();
    _stepperFerrari->movePosition(-FERRARI_POSITIONING_SPEED, calibrationPosition);
    _stepperPuller->switchModeStandby();
    _currentWinderOperation = OPERATE_STANDBY;  // TODO: Switch OPERATE_CALIBRATING ?
}

void QiMachineWinder::operateStandby() {
    logPrint(INFO, INFO, "QiMachineWinder::operateStandby\n");  // TODO: DEBUG
    _stepperSpool->switchModeStandby();
    _stepperFerrari->switchModeStandby();
    _stepperPuller->switchModeStandby();
    _currentWinderOperation = OPERATE_STANDBY;
}

void QiMachineWinder::operateOff() {
    logPrint(INFO, INFO, "QiMachineWinder::operateOff\n");  // TODO: DEBUG
    _stepperSpool->switchModeOff();
    _stepperFerrari->switchModeOff();
    _stepperPuller->switchModeOff();
    _currentWinderOperation = OPERATE_OFF;
}

void QiMachineWinder::adjustSpeed(float speedMetersPerMinute) {
    logPrint(INFO, INFO, "QiMachineWinder::adjustSpeed speedMpm: %.2f\n", speedMetersPerMinute);  // TODO: DEBUG
    speedMetersPerMinute = fixSpeedValue(speedMetersPerMinute);

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

void QiMachineWinder::adjustOscillationPositions(uint16_t positionStart, uint16_t positionEnd) {
    logPrint(INFO, INFO, "QiMachineWinder::adjustOscillationPositions\n");  // TODO: DEBUG
    // Save new positions to config
    _configurationSoft.ferrari_min = (positionStart < positionEnd) ? positionStart : positionEnd;
    _configurationSoft.ferrari_max = (positionStart < positionEnd) ? positionEnd : positionStart;

    // Adjust current operation if any
    switch (_currentWinderOperation) {
        case OPERATE_OFF:
        case OPERATE_STANDBY:
        case OPERATE_UNWINDING:
        case OPERATE_PULLING:
        case OPERATE_CALIBRATING:
            // Not calibrating anyways
            break;
        case OPERATE_WINDING:
            _stepperFerrari->adjustMovePositions(positionStart - _configurationHard.motors.ferrariOffset,
                                                 positionEnd - _configurationHard.motors.ferrariOffset);
            break;
    }
}

void QiMachineWinder::handleStatusReport() {
    if (_wifi.getConnectionMode() == CONNECTING) return;

    stepperStatus_s spoolStatus = _stepperSpool->getStatus();
    stepperStatus_s ferrariStatus = _stepperFerrari->getStatus();
    stepperStatus_s pullerStatus = _stepperPuller->getStatus();

    // Assemble report message
    StaticJsonDocument<512> doc;
    String json;
    doc["f"]["r"] = ferrariStatus.rpm;                                   // Ferrari-rpm
    doc["f"]["s"] = ferrariStatus.load;                                  // Ferrari-load
    doc["f"]["a"] = _stepperFerrari->getCurrentMode() != OFF;            // Ferrari-active
    doc["p"]["r"] = pullerStatus.rpm;                                    // Puller-rpm
    doc["p"]["s"] = pullerStatus.load;                                   // Puller-load
    doc["p"]["a"] = _stepperPuller->getCurrentMode() != OFF;             // Puller-active
    doc["s"]["r"] = spoolStatus.rpm;                                     // Spool-rpm
    doc["s"]["s"] = spoolStatus.load;                                    // Spool-load
    doc["s"]["a"] = _stepperSpool->getCurrentMode() != OFF;              // Spool-active
    doc["m"] = machineWinderOperationToString(_currentWinderOperation);  // Current machine-operation
    doc["e"] = nullptr;                                                  // Errors // TODO: Re-implement
    doc["w"] = 0;                                                        // Total windings // TODO spoolStatus.rotations;
    doc["l"] = 0;  // Total wound length // TODO ferrariStatus.rotations * hard.motors.ferrari.mm_per_rotation / 1000;

    // Serialise and check message
    if (serializeJson(doc, json) == 0) {
        Serial.println("[Winder] JSON serialization failed!");  // TODO: Debug/Logging
        // TODO: return if json failed?
    }

    // Send message
    _server.emit("stats", json);
}

float QiMachineWinder::calculateSpoolToFerrariSpeedRation() {
    const float SPOOL_GEAR_RATIO = 1;  // 5.18; // TODO - Comment - hard.motors.spool.gearRatio
    const float FILAMENT_SIZE = 1.75;  // Width of filament in mm

    float spoolWidthMm = _configurationSoft.ferrari_max - _configurationSoft.ferrari_min;  // Width of spool in mm
    float spoolMotorRotationsPerLayer =
        spoolWidthMm * SPOOL_GEAR_RATIO / FILAMENT_SIZE;  // Number of spool-rotations needed for winding an entire layer of filament
    float ferrariMotorRotationsPerLayer =
        spoolWidthMm /
        _configurationHard.motors.ferrari.mmPerRotation;  // Number of ferrari-rotations needed for winding an entire layer of filament
    float speedRatio = spoolMotorRotationsPerLayer / ferrariMotorRotationsPerLayer;

    return speedRatio;
}

void QiMachineWinder::handleSpeedAdjust() {
    if (_stepperFerrari->getCurrentMode() == OSCILLATING_FORWARD || _stepperFerrari->getCurrentMode() == OSCILLATING_BACKWARD) {
        float speedNewRpm = _stepperSpool->getStatus().rpm / calculateSpoolToFerrariSpeedRation();
        _stepperFerrari->adjustMoveSpeed(_stepperFerrari->getStatus().rpm < 0 ? min(-speedNewRpm, speedNewRpm)
                                                                              : max(-speedNewRpm, speedNewRpm));
    }
}

const char* machineWinderOperationToString(machineWinderOperation_e machineOperation) {
    switch (machineOperation) {
        case OPERATE_OFF:
            return "standby";  // TODO: Legacy, adjust and update webfrontend to something more intuitive
        case OPERATE_STANDBY:
            return "power";  // TODO: Legacy, adjust and update webfrontend to something more intuitive
        case OPERATE_PULLING:
            return "pulling";
        case OPERATE_WINDING:
            return "winding";
        case OPERATE_UNWINDING:
            return "unwinding";
        case OPERATE_CALIBRATING:
            return "calibrating";
    };
    return "error";
}

void QiMachineWinder::executeDebugCommand(char cmd) {  // TODO: To be removed after debugging
    switch (cmd) {
        case 'd':  // debug
            _stepperSpool->printStatus(true);
            _stepperFerrari->printStatus(true);
            _stepperPuller->printStatus(true);
            break;
        case 'x':
            _stepperSpool->switchModeStandby();
            _stepperFerrari->switchModeStandby();
            _stepperPuller->switchModeStandby();
            break;
        case 'X':
            _stepperSpool->switchModeOff();
            _stepperFerrari->switchModeOff();
            _stepperPuller->switchModeOff();
            break;
        case '1':
            _stepperSpool->moveRotate(60);
            _stepperPuller->moveRotate(60);
            break;
        case '2':
            _stepperSpool->moveRotate(120);
            _stepperPuller->moveRotate(60);
            break;
        case '3':
            break;
        // Ferrari control
        case 'g':
            _stepperFerrari->moveHome(60);
            break;
        case 'f':
            _stepperFerrari->moveRotate(60);
            break;
        case 'F':
            _stepperFerrari->moveRotate(-60);
            break;
        default:
            Serial.println("[DEBUG] Unknown command - input ignored");
            return;
    };
    Serial.printf("[DEBUG] executed cmd: '%c'\n", cmd);
}

uint32_t speedRpmToMpm(float speedRpm, float mmPerRotation) { return speedRpm / 1000 * mmPerRotation; }

machineWinderOperation_e QiMachineWinder::getCurrentMode() { return _currentWinderOperation; }
