#pragma once

#include <stdint.h>

/**
 * @brief Stepper hardware config that can not be changed
 *
 */
struct stepper_s {
    const char *stepperId;       // stepper identifier used in logs
    uint16_t maxCurrent;         // maximum average motor current
    uint16_t microstepsPerStep;  // subdivion count of every hard step
    float stepsPerRotation;      // hard steps per rotation (NEMA17 - 200)
    float mmPerRotation;  // mm stepper moves, mm filament pulled every rotation
    struct Pins {
        uint8_t en;    // SPI enable pin
        uint8_t dir;   // stepper direcion pin
        uint8_t step;  // stepper step pin (on/off causes step)
        uint8_t cs;    // stepper chip select pin
    } pins;
};

/**
 * @brief stepper operation modes, at every time only one mode possible
 *
 */
// TODO: Rename constans with underscore
enum mode_e {
    ROTATING,
    ADJUSTING,
    HOMING,
    POSITIONING,
    OSCILLATINGLEFT,
    OSCILLATINGRIGHT,
    STANDBY,
    OFF
};

// Manually calibrated lookup table with sorted stall values when no load
// applied (lower value means higher load)
const uint16_t minLoad[40] = {
    446, 450, 448, 447, 452, 459, 461, 465, 465, 470, 472, 473, 477, 480,
    479, 489, 490, 492, 495, 472, 505, 505, 515, 517, 526, 532, 531, 539,
    545, 579, 621, 612, 630, 630, 653, 669, 720, 794, 926, 1023};

// Manually calibrated lookup table with sorted stall values when max load
// applied (lower value means higher load)
const uint16_t maxLoad[40] = {103, 81,  89,  184, 124, 106, 129, 176, 166, 182,
                              96,  127, 167, 142, 215, 144, 197, 156, 167, 147,
                              209, 208, 185, 129, 124, 244, 133, 190, 216, 170,
                              268, 131, 296, 172, 282, 262, 281, 452, 532, 922};

// Lookup table with calibrated speeds in time US between steps, sorted from
// highest to lowest speed
const uint16_t speeds[40] = {
    90,  92,  95,  97,  100, 103, 106, 109, 113, 116,  120,  124, 129, 134,
    139, 144, 150, 157, 164, 172, 180, 190, 201, 212,  226,  241, 258, 278,
    301, 329, 361, 402, 452, 517, 603, 723, 904, 1206, 1809, 3619};

/**
 * @brief Find index of closest matching number in array via binary seach
 *
 * @param number value to seach for (needle)
 * @param arr sorted input array (haystack)
 * @param length item count in input array
 * @return uint16_t index of cosest matching number
 */
uint16_t indexOfClosestNumberInSortedArray(const uint16_t number,
                                           const uint16_t *arr,
                                           const uint8_t length);

/**
 * @brief convert mode_e in in char representation
 *
 * @param mode stepper operation mode
 * @param out memory location to write the output to
 */
void modeToString(const mode_e mode, char *out);

/**
 * @brief Ensure that number lies between two boundaries
 *
 * @param number number to set in range
 * @param lower lower range limit
 * @param upper upper range limit
 * @return uint16_t adjusted number in range
 */
uint16_t putNumberInRange(const uint16_t number, const uint16_t lower,
                          const uint16_t upper);

/**
 * @brief Convert Us between steps in rotations per minute
 *
 * @param speedUs time Us beween steps
 * @param stepsPerRotation step count within one rotation
 * @return float rotations per minute
 */
float speedUsToRpm(const int32_t speedUs, const uint32_t stepsPerRotation);

/**
 * @brief Convert rotations per minute in Us(microseconds) between two steps
 *
 * @param rpm rotations per minute
 * @param stepsPerRotation step count needed for full rotation
 * @return uint32_t time between two steps in Us(microseconds)
 */
uint32_t speedRpmToUs(float rpm, const uint32_t stepsPerRotation);

/**
 * @brief Convert raw stall to load in %
 *
 * @param speedUs time between two steps in Us(microseconds)
 * @param stall raw stall value frm stepper driver 0...1023
 * @param speeds sorted speed lookup table wit Us values
 * @param minLoad lookup table with sorted stall values when no load applied
 * @param maxLoad lookup table with sorted stall values when max load applied
 * @param length length of all lookup tables (calibration point count)
 * @return uint8_t stepper load in %, 0 = no load, 100 = full load
 */
uint8_t stallToLoadPercent(const uint16_t speedUs, const uint16_t stall,
                           const uint16_t *speeds, const uint16_t *minLoad,
                           const uint16_t *maxLoad, uint8_t length);

/**
 * @brief Convert positon in ticks to position in mm
 *
 * @param position postion in ticks as read from FastAccelStepper
 * @param stepsPerRotation step signal count for full motor rotation
 * @param mmPerRotation mm stepper moves per rotation
 * @return float position in mm
 */
float positionToMm(int32_t position, const uint32_t stepsPerRotation,
                   const float mmPerRotation);

/**
 * @brief Convert position in mm to ticks understandable by FastAccelStepper
 *
 * @param mm position in mm
 * @param stepsPerRotation step signal count for full motor rotation
 * @param mmPerRotation mm stepper moves per rotation
 * @return int32_t position in ticks understandable by FastAccelStepper
 */
int32_t mmToPosition(float mm, const uint32_t stepsPerRotation,
                     const float mmPerRotation);