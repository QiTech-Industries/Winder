#include <Arduino.h>
#include <HeatController.h>
#include <logging.h>


/**
 * Example code for using 3 heat-controllers simultaneously
 */

HeatController heater1(1, 200, 26, 16, 17, 25); // id, targetTemp, heat, so, cs, sck
HeatController heater2(2, 200, 27, 13, 12, 14); // id, targetTemp, heat, so, cs, sck
HeatController heater3(3, 200, 5, 18, 19, 23); // id, targetTemp, heat, so, cs, sck
HeatController controllerList[] = {heater1, heater2, heater3};
int controllerCount = 3;

void setup(){
    Serial.begin(115200);
    for(int i=0; i<controllerCount; ++i){
        if(controllerList[i].isReady()) controllerList[i].start();
    }
    controllerList[controllerCount-1].setDebuggingLevel(INFO);
}

void loop(){
    for(int i=0; i<controllerCount; ++i){
        if(controllerList[i].isReady()) controllerList[i].handle();
    }
    delay(1);
}