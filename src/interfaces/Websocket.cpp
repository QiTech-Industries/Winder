#include <machine/Setup.h>
// Add listeners for incoming socket events

void matchCommand(char cmd) {
    const struct commandOperation_s = { char * command;  // Identifier of the event-type, required for triggering
    std::function<String(JsonObject message)> cb;        // Callback-function, called when listener is triggered
}
const commandOperation_s commands[20] = {
    {"scan", _wifi.scan()},
    {"config", _softC.toJSON()},
    {"connect"},
    {"power"},
    {"standby"},
    {"wind"},
    {"unwind"},
    {"pull"},
    {"speed"},
    {"calibrate"},
    {"change"},
    {"modify"}
}
switch (cmd) {
    case 'scan':
        return _wifi.scan();
        break;
    case 'config':
        return _softC.asJSON();
        break;
    case 'connect':
        _wifi.connect(data["name"], data["password"]);
        return String();
        break;
    case 'power':
        operateStandby();
        return String();
        break;
    case 'standby':
        operateOff();
        return String();
        break;
    case 'wind':
        const float rpm = parseMpmToRpm(data, _hardC.motors.puller.mmPerRotation);
        operateWind(rpm);
        return String();
        break;
    // Ferrari control
    case 'unwind':
        const float rpm = parseMpmToRpm(data, _hardC.motors.puller.mmPerRotation);
        operateUnwind(rpm);
        return String();
        break;
    case 'pull':
        const float rpm = parseMpmToRpm(data, _hardC.motors.puller.mmPerRotation);
        operatePull(rpm);
        return String();
        break;
    case 'speed':
        const float rpm = parseMpmToRpm(data, _hardC.motors.puller.mmPerRotation);
        adjustSpeed(rpm);
        return String();
        break;
    case 'calibrate':
        // TODO: move 150 to hard config
        uint16_t newPositionValue = putNumberInRange(data["position"].as<uint16_t>(), 0, 150);
        bool newPositionIsStart = data["startPos"].as<bool>();

        _softC.ferrari_min = newPositionIsStart ? newPositionValue : _softC.ferrari_min;
        _softC.ferrari_max = newPositionIsStart ? _softC.ferrari_max : newPositionValue;

        _softC.store();  // Store new configuration

        // Currently moving, adjust positions smoothly to avoid interruption of ferrari-movement
        if (_currentWinderOperation == OPERATE_WINDING) {
            _stepperFerrari->adjustMovePositions(_softC.ferrari_min - _hardC.motors.ferrariOffset,
                                                 _softC.ferrari_max - _hardC.motors.ferrariOffset);
            return String();
        }

        // immediately move to the new position to make calibration easier
        operateCalibrate(newPositionValue);
        return String();
        break;
    case 'change':
        // TODO: "change" is a really unintuitive command-identifier

        operatePull(speedRpmToMpm(_stepperPuller->getStatus().rpm,
                                  _hardC.motors.puller.mmPerRotation));  // Keep pulling with same speed since we are already winding
        return String();
        break;
    case 'modify':
        // TODO: "modify" is a really unintuitive command-identifier

        _softC.wifi.ap_enabled = data["ap_enabled"];
        if (data["mdns_name"] != "") {
            strcpy(_softC.wifi.mdns_name, data["mdns_name"]);
        }
        if (data["ap_ssid"] != "") {
            strcpy(_softC.wifi.ap_ssid, data["ap_ssid"]);
        }

        // TODO: Apply wifi-related changes on the fly

        // Store new configuration
        _softC.store();  // TODO: must be called async or esp might crash on watchdog trigger

        // TODO: Manually force configuration save?
        return String("\"stored\"");
        break;
    default:
        Serial.println("[DEBUG] Unknown command - input ignored");
        return;
};
}
