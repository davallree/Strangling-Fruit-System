#ifndef INCLUDE_WALL_ANIMATION_H_
#define INCLUDE_WALL_ANIMATION_H_

#include <FastLED.h>

#include <memory>
#include <mutex>
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

// Buffer of LED data.
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

// Base class for all the patterns.
class Pattern {
 public:
  virtual void Update(LEDBuffer& buffer, uint8_t speed) = 0;
  virtual void Reset() {};
};

// All LEDs off.
class NonePattern : public Pattern {
 public:
  void Update(LEDBuffer& buffer, uint8_t speed) override {
    fill_solid(buffer.raw_led_data(), buffer.num_leds(), CRGB::Black);
  }
};

class SpiralPattern : public Pattern {
 public:
  void Update(LEDBuffer& buffer, uint8_t speed) override {
    uint8_t rotation = beat8(speed);
    for (LED& led : buffer.leds()) {
      uint8_t brightness =
          sin8(twist_ * led.radius() + strands_ * led.angle() - rotation);
      led.color().setHSV(212, 255, brightness);
    }
  }

 private:
  uint8_t twist_ = 2;
  uint8_t strands_ = 4;
};

class WavePattern : public Pattern {
 public:
  enum class Direction { kIn, kOut };

  WavePattern(Direction direction) { SetDirection(direction); }

  void Update(LEDBuffer& buffer, uint8_t speed) override {
    // Divide speed by 2, otherwise wave looks faster.
    uint8_t wave_phase = (direction_ * beat8(speed)) / 2;

    for (LED& led : buffer.leds()) {
      uint8_t brightness = sin8(scale_ * (led.radius() + wave_phase));
      led.color().setHSV(212, 255, brightness);
    }
  }

  void SetDirection(Direction direction) {
    direction_ = direction == Direction::kIn ? 1 : -1;
  }

 private:
  uint8_t scale_ = 2;
  int8_t direction_ = 1;
};

class RosePattern : public Pattern {
 public:
  void Update(LEDBuffer& buffer, uint8_t speed) override {
    uint8_t rotation = beat8(speed);
    uint8_t ripple = beat8(speed);
    for (LED& led : buffer.leds()) {
      uint8_t brightness =
          sin8(zoom_ * led.radius() +
               shape_ * sin8(petals_ * led.angle() + rotation) + ripple);
      led.color().setHSV(212, 255, brightness);
    }
  }

 private:
  uint8_t zoom_ = 1;
  uint8_t shape_ = 1;
  uint8_t petals_ = 3;
};

class CirclesPattern : public Pattern {
 public:
  void Update(LEDBuffer& buffer, uint8_t speed) override {
    uint8_t x_translation = 0;
    uint8_t y_translation = 0;
    uint8_t warp = beat8(speed);

    for (LED& led : buffer.leds()) {
      uint8_t brightness = sin8(sin8(scale_ * led.x() + x_translation) +
                                sin8(scale_ * led.y() + y_translation) + warp);
      led.color().setHSV(212, 255, brightness);
    }
  }

 private:
  uint8_t scale_ = 1;
};

class AwaitTouchPattern : public Pattern {
 public:
  void Update(LEDBuffer& buffer, uint8_t speed) override {
    uint8_t wave_phase = beatsin8(speed, 32, 64);

    for (LED& led : buffer.leds()) {
      if (led.radius() < 128) {
        uint8_t r = 2 * (led.radius() + wave_phase);
        uint8_t brightness = cos8(r);
        led.color().setHSV(212, 255, brightness);
      } else {
        led.color() = CRGB::Black;
      }
    }
  }

 private:
  uint8_t scale_ = 1;
};

class GlitchPattern : public Pattern {
 public:
  void Update(LEDBuffer& buffer, uint8_t speed) override {
    if (millis() > next_glitch_time_millis_) {
      randSeed_ = millis();
      next_glitch_time_millis_ =
          millis() + random(100, lerp16by8(100, 4000, speed));
    }
    randomSeed(randSeed_);
    // Create a mostly static pattern with blue
    for (int i = 0; i < buffer.num_leds(); ++i) {
      int randVal = random(100);
      buffer.led_data()[i] = CRGB::Blue;
      if (randVal > 98) {
        // Large glitch segments
        int glitchLength = random(5, 30);
        for (int j = 0; j < glitchLength && (i + j) < buffer.num_leds(); j++) {
          buffer.led_data()[i + j] = random(2) == 0 ? CRGB::Black : CRGB::White;
        }
        i += glitchLength;  // Skip over the glitch segment
      } else if (randVal > 90) {
        // Small glitch pixels
        buffer.led_data()[i] = random(2) == 0 ? CRGB::Black : CRGB::White;
      }
    }
  }

 private:
  int randSeed_ = 0;
  uint64_t next_glitch_time_millis_ = 0;
};

class ClimaxPattern : public Pattern {
 public:
  void Update(LEDBuffer& buffer, uint8_t speed) override {
    rand16seed = rand_seed_;
    uint32_t current_time = millis();
    uint32_t elapsed_time = current_time - start_time_;

    // Calculate the appropriate fill_progress_ based on the elapsed time
    if (elapsed_time < duration_) {
      fill_progress_ = (255 * elapsed_time) / duration_;
    } else {
      fill_progress_ = 255;
    }

    for (LED& led : buffer.leds()) {
      // Randomly decide if this LED should start brightening
      if (random8() < fill_progress_) {
        led.color() += CHSV(10, 10, 1);
      } else {
        // Keep the LED off (black) until it's chosen to brighten
        led.color().setHSV(0, 0, 0);
      }
    }
  }

  void Reset() override {
    rand_seed_ = millis();
    fill_progress_ = 0;
    start_time_ = millis();  // Reset the start time
  }

 private:
  uint16_t rand_seed_ = 0;
  uint8_t fill_progress_ = 0;
  uint32_t start_time_ = 0;
  uint32_t duration_ = 20'000;  // Default duration (in milliseconds)
  uint8_t scale_ = 2;
};

class RecoveryPattern : public Pattern {
 public:
  void Update(LEDBuffer& buffer, uint8_t speed) override {
    uint8_t offset = beatsin8(speed / 2);
    uint8_t pulse = beatsin8(speed / 2, 128, 255);
    for (LED& led : buffer.leds()) {
      uint8_t a = qadd8(led.y(), offset);
      if (a < 128) a = 128;
      uint8_t brightness = cos8(a);
      brightness = scale8_video(brightness, pulse);
      led.color().setHSV(212, 255, brightness);
    }
  }
};

class ManBurnPattern : public Pattern {
 public:
  void Update(LEDBuffer& buffer, uint8_t speed) override {
    uint8_t rotation = beat8(speed);
    uint8_t ripple = beat8(speed);
    for (LED& led : buffer.leds()) {
      uint8_t brightness =
          sin8(zoom_ * led.radius() +
               shape_ * sin8(petals_ * led.angle() + rotation) + ripple);
      // Change the hue slightly based on the distance for a more psychedelic
      // effect
      uint8_t hue = 212 + (led.radius() / 2);

      led.color().setHSV(hue, 255, brightness);
    }
  }

 private:
  uint8_t zoom_ = 1;
  uint8_t shape_ = 1;
  uint8_t petals_ = 3;
  uint8_t scale_ = 1;
};

class TempleBurnPattern : public Pattern {
 public:
  void Update(LEDBuffer& buffer, uint8_t speed) override {
    uint8_t offset = beat8(speed);
    for (LED& led : buffer.leds()) {
      uint8_t brightness = sin8(scale8(255 - led.x(), led.y()) +
                                scale8(255 - led.y(), led.x()) + offset);
      led.color().setHSV(0, 0, brightness);
    }
  }
};

// Controls the LED matrix.
class LEDController {
 public:
  LEDController();

  // Initialize the LEDs with data from LED Mapper.
  void InitLEDs(int num_leds, const std::vector<uint8_t> coordsX,
                const std::vector<uint8_t> coordsY,
                const std::vector<uint8_t> angles,
                const std::vector<uint8_t> radii);

  // Sets the current pattern to show on the LED matrix. The transition duration
  // determines how long the pattern will blend with the previous pattern.
  // Locks excluded: mu_.
  void SetCurrentPattern(PatternId pattern_id, uint8_t pattern_speed,
                         int transition_duration_millis);

  // Call from loop().
  // Locks excluded: mu_.
  void Update();

  // Buffer for FastLED data.
  std::vector<CRGB>& led_data() { return led_buffer_.led_data(); }

  PatternId current_pattern_id() const { return current_pattern_id_; }

  void set_enabled(bool enabled) {
    enabled_ = enabled;
    // Set LED buffer to black if we want them to be off.
    if (!enabled_) {
      for (LED& led : led_buffer_.leds()) {
        led.color() = CRGB::Black;
      }
    }
  }

 private:
  void InitBuffers(int num_leds);

  // Protects members from concurrent access.
  std::mutex mu_;

  bool enabled_ = true;

  std::vector<std::unique_ptr<Pattern>> patterns_;

  // The buffer that FastLED points to.
  LEDBuffer led_buffer_;
  // This buffer isn't connected to FastLED. It is used to blend the previous
  // animation into the current one.
  LEDBuffer previous_buffer_;

  // Time at which the current pattern started playing.
  uint64_t transition_start_millis_ = 0;
  // Duration of the pattern transition.
  uint64_t transition_duration_millis_ = 0;

  // TODO: create a struct for that crap.
  PatternId previous_pattern_id_ = PatternId::kNone;
  uint8_t previous_pattern_speed_ = 60;
  PatternId current_pattern_id_ = PatternId::kRecovery;
  uint8_t current_pattern_speed_ = 60;
};

#endif  // INCLUDE_WALL_ANIMATION_H_
