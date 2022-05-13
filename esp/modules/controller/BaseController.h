#pragma once
#include <stdint.h>

#include "../logger/logging.h"
#include "../validator/McValidator.h"
// #include "../validator/McValidatorEsp32.h"

/**
 * @brief Abstract class for controlling active modules
 */
class BaseController{
    protected:
        loggingLevel_e _logging = NONE; // Logging level, no logging by default
        McValidator _mcValidator = McValidator(); // Microcontroller-Validator for checking pin setup
    public:
        // TODO
        // void logPrint(loggingLevel_e currentLevel, loggingLevel_e messageLevel, char* message, ...);

        /**
         * @brief Initialises the controller, for example by setting pins
         */
        virtual void init();
        
        /**
         * @brief Called repeatedly, handles states and changes
         */
        virtual void handle();

        /**
         * @brief Check whether controller was initialised and is in a valid state
         * 
         * @return true controller initialised and ready
         * @return false controller not ready
         */
        virtual bool isReady();

        // Setter-method
        void setDebuggingLevel(loggingLevel_e level);
};