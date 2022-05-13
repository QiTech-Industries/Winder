#include <StepperTest.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// Implements binary search
uint16_t indexOfClosestNumberInSortedArray(const uint16_t number,
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

void modeToString(const mode_e mode, char *out) {
    const char *states[8] = {
        "ROTATING",        "ADJUSTING",        "HOMING", "POSITIONING",
        "OSCILLATINGLEFT", "OSCILLATINGRIGHT", "STANDBY",  "OFF"};
    strcpy(out, states[mode]);
}

uint16_t putNumberInRange(const uint16_t number, const uint16_t lower,
                          const uint16_t upper) {
    if (upper < number) return upper;
    if (lower > number) return lower;
    return number;
}

float speedUsToRpm(const int32_t speedUs, const uint32_t stepsPerRotation) {
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

uint8_t stallToLoadPercent(const uint16_t speedUs, const uint16_t stall,
                           const uint16_t *speeds, const uint16_t *minLoad,
                           const uint16_t *maxLoad, uint8_t length) {
    // maxLoad values are always lower than minLoad values
    uint8_t index = indexOfClosestNumberInSortedArray(speedUs, speeds, length);
    uint16_t min = minLoad[index];
    uint16_t max = maxLoad[index];
    uint16_t normedStall = putNumberInRange(stall, max, min);
    return round((min - normedStall) * 100 / (min - max));
};

float positionToMm(int32_t position, const uint32_t stepsPerRotation,
                   const float mmPerRotation) {
    if (mmPerRotation == 0) return 0;
    return position * mmPerRotation / stepsPerRotation;
}

int32_t mmToPosition(float mm, const uint32_t stepsPerRotation,
                     const float mmPerRotation) {
    if (mmPerRotation == 0) return 0;
    return mm * stepsPerRotation / mmPerRotation;
}