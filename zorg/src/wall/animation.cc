#include "wall/animation.h"

#include <pixeltypes.h>

#include <cmath>
#include <vector>

void LEDController::InitLEDs(int num_leds, const uint8_t* coordsX,
                             const uint8_t* coordsY, const uint8_t* angles,
                             const uint8_t* radii) {
  InitLedBuffer(num_leds);
  leds_.reserve(led_buffer_.size());
  for (int i = 0; i < led_buffer_.size(); ++i) {
    leds_.push_back(
        LED(led_buffer_[i], coordsX[i], coordsY[i], angles[i], radii[i]));
  }
}

template <typename T>
uint8_t map_to_256(T l, T inMin, T inMax) {
  uint8_t outMin = 0;
  uint8_t outMax = 255;
  if (inMax - inMin + outMin == 0) return 0;

  return ((l - inMin) * (outMax - outMin)) / (inMax - inMin) + outMin;
}

void LEDController::InitLEDs(int sizeX, int sizeY) {
  InitLedBuffer(sizeX * sizeY);
  float centerX = sizeX / 2;
  float centerY = sizeY / 2;
  leds_.reserve(led_buffer_.size());

  float max_radius = std::max(centerX, centerY);
  for (int y = 0; y < sizeY; ++y) {
    for (int x = 0; x < sizeX; ++x) {
      uint8_t x256 = map_to_256(x, 0, sizeX);
      uint8_t y256 = map_to_256(y, 0, sizeY);
      float angle = atan2(centerY - y, centerX - x);
      uint8_t angle256 = map_to_256(angle, 0.f, (float)(2 * PI));
      float radius = hypot(x - centerX, y - centerY);
      uint8_t radius256 = map_to_256(radius, 0.f, max_radius);
      leds_.push_back(
          LED(led_buffer_[(y * sizeY) + x], x256, y256, angle256, radius256));
    }
  }
}

void rainbow(std::vector<CRGB>& leds) {
  fill_rainbow(leds.data(), leds.size(), beat8(33));
}

void pulse(std::vector<CRGB>& leds) {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (CRGB& led : leds) {
    led = ColorFromPalette(palette, 0, beat);
  }
}

// List of patterns to cycle through during ambient state.  Each is defined as a
// separate function below.
typedef void (*AmbientPattern)(std::vector<CRGB>&);
// Index number of which pattern is current.

// The ambient animation cycles through a set of patterns.
void ambient_animation(std::vector<CRGB>& leds) {
  static uint8_t current_pattern = 0;
  static const std::vector<AmbientPattern> ambient_patterns = {rainbow, pulse};
  // Call the current pattern's function.
  ambient_patterns[current_pattern](leds);

  // Go to next pattern every N seconds.
  EVERY_N_SECONDS(5) {
    current_pattern = (current_pattern + 1) % ambient_patterns.size();
  }
}

void touched_animation(std::vector<CRGB>& leds) {
  uint8_t beat = beatsin8(200, 0);
  for (CRGB& led : leds) {
    led.setHSV(0, 0, beat);
  }
}

void glitch_animation(std::vector<CRGB>& leds) {
  EVERY_N_MILLIS(500) {
    for (CRGB& led : leds) {
      led.setRGB(random8(), random8(), random8());
    }
  }
}

void climax_animation(std::vector<CRGB>& leds) {
  fill_rainbow(leds.data(), leds.size(), beat8(160));
  fade_video(leds.data(), leds.size(), beat8(60));
}
