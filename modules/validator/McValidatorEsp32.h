#pragma once
#include "McValidator.h"

/**
 * @brief Validator-Implementation for the ESP-32 (TODO: Check full mc-id)
 */
class McValidatorEsp32 : public virtual McValidator{
    public:
        /**
         * @brief Checks whether a digital pin is valid and exists
         * 
         * @param pin Pin-number
         * @return true pin exists
         * @return false pin does not exist
         */
        bool isDigitalPinValid(uint8_t pin);
        
        /**
         * @brief Checks whether an analog pin is valid and exists
         * 
         * @param pin Pin-number
         * @return true pin exists
         * @return false pin does not exist
         */
        bool isAnalogPinValid(uint8_t pin);
};