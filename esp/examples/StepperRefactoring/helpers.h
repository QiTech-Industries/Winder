#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>

// stepper setting that can't be modified by methods
struct stepper_s {
    const char *stepperId;
    uint16_t maxCurrent;
    uint16_t microstepsPerStep;
    float stepsPerRotation;
    float mmPerRotation;
    struct Pins {
        uint8_t en;
        uint8_t dir;
        uint8_t step;
        uint8_t cs;
    } pins;
};

enum mode_e {
    ROTATING,
    ADJUSTING,
    HOMING,
    POSITIONING,
    OSCILLATINGLEFT,
    OSCILLATINGRIGHT,
    POWER,
    OFF
};

// Lookup reference Tables for load calculation
const uint16_t minLoad[40] = {
    446, 450, 448, 447, 452, 459, 461, 465, 465, 470, 472, 473, 477, 480,
    479, 489, 490, 492, 495, 472, 505, 505, 515, 517, 526, 532, 531, 539,
    545, 579, 621, 612, 630, 630, 653, 669, 720, 794, 926, 1023};

const uint16_t maxLoad[40] = {103, 81,  89,  184, 124, 106, 129, 176, 166, 182,
                              96,  127, 167, 142, 215, 144, 197, 156, 167, 147,
                              209, 208, 185, 129, 124, 244, 133, 190, 216, 170,
                              268, 131, 296, 172, 282, 262, 281, 452, 532, 922};

const uint16_t speeds[40] = {
    90,  92,  95,  97,  100, 103, 106, 109, 113, 116,  120,  124, 129, 134,
    139, 144, 150, 157, 164, 172, 180, 190, 201, 212,  226,  241, 258, 278,
    301, 329, 361, 402, 452, 517, 603, 723, 904, 1206, 1809, 3619};

// Implements binary search
uint16_t indexOfClosestNumberInSortedArray(const uint16_t number,
                                           const uint16_t *arr,
                                           const uint8_t length);

void modeToString(const mode_e mode, char *out);

uint16_t putNumberInRange(const uint16_t number, const uint16_t lower,
                          const uint16_t upper);

float speedUsToRpm(const uint32_t speedUs, const uint32_t stepsPerRotation);

uint32_t speedRpmToUs(float rpm, const uint32_t stepsPerRotation);

uint8_t stallToLoadPercent(const uint16_t speedUs, const uint16_t stall,
                           const uint16_t *speeds, const uint16_t *minLoad,
                           const uint16_t *maxLoad, uint8_t length);

float positionToMm(int32_t position, const uint32_t stepsPerRotation,
                   const float mmPerRotation);

int32_t mmToPosition(float mm, const uint32_t stepsPerRotation,
                     const float mmPerRotation);

#endif