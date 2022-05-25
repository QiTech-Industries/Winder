#include <Utils.h>
#include <stdint.h>

float putNumberInRange(const float number, const float lower, const float upper) {
    if (upper < number) return upper;
    if (lower > number) return lower;
    return number;
}

float mpmToRpm(const float mpm, const float mmPerRotation) { return mpm * 1000 / mmPerRotation; }
