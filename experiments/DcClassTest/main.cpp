/**
 * @file main.cpp
 * @author Simon Kolb (simon@qitech.de)
 * @brief Example meant to demonstrate and test some functions of the dcMotor class implementation
 * @version 0.1
 * @date 2022-07-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>

#include "controller/dcmotor/DcMotor.h"

DcMotor dcMotor = DcMotor({.motorId = "Puller",
                           .ticksPerRotation = 2100,
                           .pins{
                               .rightTurn = 23,
                               .leftTurn = 22,
                               .encoderA = 26,
                               .encoderB = 27,
                           }});

void callInterrupt() { dcMotor.handleInterrupt(); };

void setup() {
    dcMotor.init(callInterrupt);
    dcMotor.turnRightPwm(100);
}

void loop() {
    dcMotor.handle();
    if (dcMotor.getPosition() > 21000) dcMotor.brake();
    if (!dcMotor.isMoving() && dcMotor.getPosition() != 0){
        dcMotor.resetPosition();
        dcMotor.turnLeftPwm(255);
    }
    if (dcMotor.getPosition() < -10000) dcMotor.brake();
}
