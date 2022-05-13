#include "McValidatorEsp32.h"

#include <stdint.h>

bool McValidatorEsp32::isDigitalPinValid(uint8_t pin){
    switch(pin){
        // SPI-pins
            case 18:
            case 19:
            case 23:
            case 05:
        // Digital pins
            case 13:
            case 12:
            case 14:
            case 27:
            case 16:
            case 17:
            case 25:
            case 26:
                return true;
        // Analog pins:
            case 2:
            case 4:
            case 36:
            case 34:
            case 38:
            case 39:
            default:
                return false;
    }
}

bool McValidatorEsp32::isAnalogPinValid(uint8_t pin){
    switch(pin){
        // SPI-pins
            case 18:
            case 19:
            case 23:
            case 05:
        // Digital pins
            case 13:
            case 12:
            case 14:
            case 27:
            case 16:
            case 17:
            case 25:
            case 26:
                return false;
        // Analog pins:
            case 2:
            case 4:
            case 36:
            case 34:
            case 38:
            case 39:
            default:
                return true;
    }
}