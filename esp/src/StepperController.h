#pragma once
#include "../modules/controller/BaseController.h"

class StepperController : BaseController{
    public:
        // Constructor
        StepperController();
        
        /**
         * @brief Initialises the controller, for example by setting pins
         */
        void init();
        
        /**
         * @brief Called repeatedly, handles states and changes
         */
        void handle();

        /**
         * @brief Check whether controller was initialised and is in a valid state
         * 
         * @return true controller initialised and ready
         * @return false controller not ready
         */
        bool isReady();
};