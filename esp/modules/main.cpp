#include <Arduino.h>

#include "controller/heater/HeatController.h"
#include "logger/logging.h"

/**
 * Example code for using 3 heat-controllers simultaneously
 */

HeatController heater1({
    .id = 1,
    .targetTemp = 200,
    .pinHeat = 26,
    .pinSensorSo = 16,
    .pinSensorCs = 17,
    .pinSensorSck = 25
});
HeatController heater2({
    .id = 2,
    .targetTemp = 200,
    .pinHeat = 27,
    .pinSensorSo = 13,
    .pinSensorCs = 12,
    .pinSensorSck = 14
});
HeatController heater3({
    .id = 3,
    .targetTemp = 200,
    .pinHeat = 5,
    .pinSensorSo = 18,
    .pinSensorCs = 19,
    .pinSensorSck = 23
});
HeatController controllerList[] = {heater1, heater2, heater3};
int controllerCount = 3;

void setup(){
    Serial.begin(115200);
    for(int i=0; i<controllerCount; ++i){
        if(controllerList[i].isReady()) controllerList[i].start();
    }
    controllerList[0].setDebuggingLevel(WARNING);
    controllerList[controllerCount-1].setDebuggingLevel(INFO);
}

void loop(){
    for(int i=0; i<controllerCount; ++i){
        if(controllerList[i].isReady()) controllerList[i].handle();
    }
    delay(1);
}