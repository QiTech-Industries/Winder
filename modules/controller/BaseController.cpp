#include "BaseController.h"

#include <stdint.h>

void BaseController::setDebuggingLevel(loggingLevel_e level){
    _logging = level;
}