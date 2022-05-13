#pragma once
#include <stdint.h>

/**
 * @brief Validator for a microcontroller, for example for checking whether a pin exists and is of a specified type
 */
class McValidator{
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