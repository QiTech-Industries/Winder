#include "BaseController.h"

#include <stdint.h>

bool BaseController::isDigitalPinValid(uint8_t pin){
    return _mcValidator.isDigitalPinValid(pin);
}

void BaseController::setDebuggingLevel(loggingLevel_e level){
    _logging = level;
}