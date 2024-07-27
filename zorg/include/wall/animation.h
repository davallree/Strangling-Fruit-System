#ifndef INCLUDE_WALL_ANIMATION_H_
#define INCLUDE_WALL_ANIMATION_H_

#include <FastLED.h>

#include <vector>

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

class LEDController {
 public:
  void InitLEDs(int num_leds, const uint8_t* coordsX, const uint8_t* coordsY,
                const uint8_t* angles, const uint8_t* radii);
  void InitLEDs(int sizeX, int sizeY);

  std::vector<LED>& leds() { return leds_; }

  // Buffer for FastLED data.
  std::vector<CRGB>& led_buffer() { return led_buffer_; }

 private:
  void InitLedBuffer(int num_leds) {
    led_buffer_.assign(num_leds, CRGB::Black);
  }
  // The buffer that FastLED points to.
  std::vector<CRGB> led_buffer_;
  std::vector<LED> leds_;
};

inline void pulse(LEDController& controller) {
  for (LED& led : controller.leds()) {
    led.color().setHSV(led.angle(), 100, 50);
  }
}

void rainbow(std::vector<CRGB>& leds);

void pulse(std::vector<CRGB>& leds);

// The ambient animation cycles through a set of patterns.
void ambient_animation(std::vector<CRGB>& leds);

void touched_animation(std::vector<CRGB>& leds);

void glitch_animation(std::vector<CRGB>& leds);

void climax_animation(std::vector<CRGB>& leds);

// Implementation

#endif  // INCLUDE_WALL_ANIMATION_H_
