void matchCommand(char cmd) {
    const commandOperation_s commands[20] = {
    {"d", _wifi.scan()},
    {"x", _softC.toJSON()},
    {"X"},
    {"1"},
    {"2"},
    {"3"},
    {"g"},
    {"f"},
    {"F"}
}
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
