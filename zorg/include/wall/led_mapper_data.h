// This file contains the LED Mapper data.
// To use the actual wall data, set the ACTUAL_WALL preprocessor variable when
// building the binary (-DACTUAL_WALL).
#ifndef INCLUDE_WALL_LED_MAPPER_DATA_H_
#define INCLUDE_WALL_LED_MAPPER_DATA_H_

#include <cstdint>
#include <vector>

// Number of LEDs.
extern const int kNumLeds;

// X coordinate, between 0-255.
extern std::vector<uint8_t> coordsX;
// Y coordinate, between 0-255.
extern std::vector<uint8_t> coordsY;
// Angle from center, between 0-255.
extern std::vector<uint8_t> angles;
// Radius from center, between 0-255.
extern std::vector<uint8_t> radii;

#endif  // INCLUDE_WALL_LED_MAPPER_DATA_H_
