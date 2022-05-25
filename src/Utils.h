#pragma once

/**
 * @brief Set number in the range of two given boundaries
 *
 * @param number number to be set in range
 * @param lower lower boundary
 * @param upper upper boundary
 * @return float closest matching boundary input number itself
 */
float putNumberInRange(const float number, const float lower, const float upper);

/**
 * @brief Convert speed from meters per minute to rotations per minute
 * 
 * @param mpm speed in meters per minute
 * @param mmPerRotation millimeters moved per rotation
 * @return float speed in rotations per minute
 */
float mpmToRpm(const float mpm, const float mmPerRotation);
