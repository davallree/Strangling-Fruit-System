#ifndef INCLUDE_WALL_ANIMATION_H_
#define INCLUDE_WALL_ANIMATION_H_

#include <FastLED.h>

#include <vector>

void rainbow(std::vector<CRGB>& leds);

void pulse(std::vector<CRGB>& leds);

// The ambient animation cycles through a set of patterns.
void ambient_animation(std::vector<CRGB>& leds);

void touched_animation(std::vector<CRGB>& leds);

void glitch_animation(std::vector<CRGB>& leds);

#endif  // INCLUDE_WALL_ANIMATION_H_
