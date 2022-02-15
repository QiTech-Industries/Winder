#include <Arduino.h>
#include <HeatController.h>

HeatController heater1(1, (double) 200, 26, 16, 17, 25); // heat, so, cs, sck
HeatController heater2(2, (double) 200, 27, 13, 12, 14); // heat, so, cs, sck
HeatController heater3(3, (double) 200, 5, 18, 19, 23); // heat, so, cs, sck
HeatController controllerList[] = {heater1, heater2, heater3};
int controllerCount = 3;

void setup(){
    Serial.begin(115200);
    for(int i=0; i<controllerCount; ++i){
        if(controllerList[i].isReady()) controllerList[i].start();
    }
}

void loop(){
    for(int i=0; i<controllerCount; ++i){
        if(controllerList[i].isReady()) controllerList[i].handleStates();
    }
    
    delay(1);
}