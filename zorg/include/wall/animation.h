#ifndef INCLUDE_WALL_ANIMATION_H_
#define INCLUDE_WALL_ANIMATION_H_

#include <FastLED.h>

#include <vector>

#include "common/messages.h"

// A single LED light. Use x(), y(), angle() and radius() to get a value between
// 0 and 255.
class LED {
 public:
  LED(CRGB& data, uint8_t x, uint8_t y, uint8_t angle, uint8_t radius)
      : data_(data), x_(x), y_(y), angle_(angle), radius_(radius) {}
  CRGB& color() { return data_; }
  uint8_t x() const { return x_; }
  uint8_t y() const { return y_; }
  uint8_t angle() const { return angle_; }
  uint8_t radius() const { return radius_; }

 private:
  CRGB& data_;
  uint8_t x_;
  uint8_t y_;
  uint8_t angle_;
  uint8_t radius_;
};

class LEDBuffer {
 public:
  void Init(int size) { led_data_.assign(size, CRGB::Black); }
  void AddLED(uint8_t x, uint8_t y, uint8_t angle, uint8_t radius) {
    leds_.push_back(LED(led_data_[leds_.size()], x, y, angle, radius));
  }
  std::vector<LED>& leds() { return leds_; }
  std::vector<CRGB>& led_data() { return led_data_; }
  CRGB* raw_led_data() { return led_data_.data(); }
  int num_leds() { return led_data_.size(); }

 private:
  std::vector<LED> leds_;
  std::vector<CRGB> led_data_;
};

// Controls the LED matrix.
class LEDController {
 public:
  static constexpr int kDefaultTransitionDurationMillis = 1000;
  // Ambient patterns take in the LEDs, and a blend factor.
  typedef void (*AmbientPattern)(LEDBuffer&);

  LEDController();

  // Initialize the LEDs with data from LED Mapper.
  void InitLEDs(int num_leds, const uint8_t* coordsX, const uint8_t* coordsY,
                const uint8_t* angles, const uint8_t* radii);
  // Initialize the LEDs with a grid of LEDs in alternating directions.
  void InitLEDs(int sizeX, int sizeY);

  // Set the animation that should be currently playing.
  void SetCurrentAnimation(WallAnimation animation);

  // Call from loop().
  void Update();

  std::vector<LED>& leds() { return led_buffer_.leds(); }

  // Buffer for FastLED data.
  std::vector<CRGB>& led_data() { return led_buffer_.led_data(); }

  WallAnimation current_animation() const { return current_animation_; }

 private:
  void InitBuffers(int num_leds);
  // Sets the current pattern to show on the LED matrix. The transition duration
  // determines how long the pattern will blend with the previous pattern.
  void SetCurrentPattern(
      AmbientPattern pattern,
      int transition_duration_millis = kDefaultTransitionDurationMillis);

  // The buffer that FastLED points to.
  LEDBuffer led_buffer_;
  // This buffer isn't connected to FastLED. It is used to blend the previous
  // animation into the current one.
  LEDBuffer previous_buffer_;

  // The animation currently playing.
  WallAnimation current_animation_ = WallAnimation::kAmbient;

  // Ambient patterns.
  static void OutwardWave(LEDBuffer& buffer);
  static void InwardWave(LEDBuffer& buffer);
  static void RainbowHorizontal(LEDBuffer& buffer);
  static void RainbowVertical(LEDBuffer& buffer);

  // Other patterns.
  static void TouchedPattern(LEDBuffer& buffer);

  //  Patterns that the ambient animation cycles through.
  const std::vector<AmbientPattern> ambient_patterns = {
      TouchedPattern, RainbowVertical, OutwardWave, InwardWave,
      RainbowHorizontal};
  // Index into ambient_patterns.
  uint8_t current_ambient_pattern_ = 0;

  // Time at which the current pattern started playing.
  uint64_t transition_start_millis_;
  // Duration of the pattern transition.
  uint64_t transition_duration_millis_;
  // Previous pattern, can be null.
  AmbientPattern previous_pattern_;
  // Current pattern, should never be null.
  AmbientPattern current_pattern_;
};

// Implementation

#endif  // INCLUDE_WALL_ANIMATION_H_
