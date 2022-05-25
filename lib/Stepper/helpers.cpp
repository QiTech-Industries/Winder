#include <stdint.h>

// stepper setting that can't be modified by methods
struct stepper_s {
    char stepperId;
    uint16_t maxCurrent;
    uint16_t microstepsPerStep;
    float stepsPerRotation;
    uint16_t microsteps = microstepsPerStep * stepsPerRotation;
    float mmPerRotation;
    struct Pins {
        uint8_t en;
        uint8_t dir;
        uint8_t step;
        uint8_t cs;
    } pins;
};

enum mode_e { ROTATING, ADJUSTING, HOMING, MOVEPOS1, MOVEPOS2, POWER, OFF };

// Lookup reference Tables for load calculation
const uint16_t minLoad[40]{989, 795, 680, 574, 533, 529, 477, 519, 452, 503,
                           442, 435, 423, 398, 436, 392, 388, 405, 406, 406,
                           364, 386, 380, 392, 374, 391, 367, 374, 358, 347,
                           374, 365, 369, 351, 341, 343, 359, 328, 327, 332};
const uint16_t maxLoad[40]{989, 795, 680, 574, 533, 529, 477, 519, 452, 503,
                           442, 435, 423, 398, 436, 392, 388, 405, 406, 406,
                           364, 386, 380, 392, 374, 391, 367, 374, 358, 347,
                           374, 365, 369, 351, 341, 343, 359, 328, 327, 332};
const uint16_t speeds[40]{3619, 1809, 1206, 904, 723, 603, 517, 452, 402, 361,
                          329,  301,  278,  258, 241, 226, 212, 201, 190, 180,
                          172,  164,  157,  150, 144, 139, 134, 129, 124, 120,
                          116,  113,  109,  106, 103, 100, 97,  95,  92,  90};

// Implements binary search
uint16_t indexOfClosestNumberInSortedArray(const float number,
                                           const uint16_t *arr,
                                           const uint8_t length) {
    if (number < arr[0]) return 0;
    if (number > arr[length - 1]) return length - 1;

    uint16_t lo = 0;
    uint16_t hi = length - 1;

    while (lo <= hi) {
        uint16_t mid = (hi + lo) / 2;

        if (number < arr[mid]) {
            hi = mid - 1;
        } else if (number > arr[mid]) {
            lo = mid + 1;
        } else {
            return mid;
        }
    }
    // if number is not in array find smallest difference
    // if difference is equal lower index is returned
    return (arr[lo] - number) < (number - arr[hi]) ? lo : hi;
}

void modeToString(const mode_e mode, const char *out) {
    switch (mode) {
        case POWER:
            out = "POWER";
            break;
        case ADJUSTING:
            out = "ADJUSTING";
            break;
        case OFF:
            out = "OFF";
            break;
        case ROTATING:
            out = "ROTATING";
            break;
        case HOMING:
            out = "HOMING";
            break;
        case MOVEPOS1:
            out = "MOVEPOS1";
            break;
        case MOVEPOS2:
            out = "MOVEPOS2";
            break;
    }
}

// maximum one switch happens
mode_e switchMode(mode_e currentMode, mode_e targetMode, bool motorStopped,
                  bool homed, bool oscillating, uint8_t load) {
    // oscillate bewtween POS1 and POS2
    if (currentMode == MOVEPOS1 && motorStopped && oscillating) {
        return MOVEPOS2;
    }
    if (currentMode == MOVEPOS2 && motorStopped && oscillating) {
        return MOVEPOS1;
    }

    // Auto switch to POWER on end of finite states
    // switch from infinite state does not go thrue POWER state
    if (currentMode != OFF && motorStopped) {
        return POWER;
    }
    if (currentMode == HOMING && load == 100) {
        homed = true;
        return POWER;
    }

    // no switching needed
    if (currentMode == targetMode) return currentMode;

    // switch to home mode if target mode needs home
    if (!homed && (targetMode == MOVEPOS1 || targetMode == MOVEPOS2)) {
        return HOMING;
    }

    return targetMode;
}

uint16_t putNumberInRange(const uint16_t number, const uint16_t lower,
                          const uint16_t upper) {
    if (upper < number) return upper;
    if (lower > number) return lower;
    return number;
}

float speedUsToRpm(const uint32_t speedUs, const uint32_t stepsPerRotation) {
    // 1 minute has 60 million microseconds
    // .0 for percise float calculation
    if (speedUs == 0 || stepsPerRotation == 0) return 0;
    return 60000000.0 / speedUs / stepsPerRotation;
}

uint32_t speedRpmToUs(float rpm, const uint32_t stepsPerRotation) {
    // 1 minute has 60 million microseconds
    if (rpm == 0 || stepsPerRotation == 0) return 0;
    if (rpm < 0) rpm = rpm * -1;
    return 60000000 / rpm / stepsPerRotation;
}

uint8_t stallToLoadPercent(const uint16_t stall, const float rpm) {
    // maxLoad values are always lower than minLoad values
    uint8_t index = indexOfClosestNumberInSortedArray(rpm, speeds, 40);
    uint16_t min = minLoad[index];
    uint16_t max = maxLoad[index];
    uint16_t normedStall = putNumberInRange(stall, max, min);
    return round((min - normedStall) * 100 / (min - max));
};