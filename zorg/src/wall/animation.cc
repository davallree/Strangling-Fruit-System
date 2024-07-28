#include "wall/animation.h"

#include <pixeltypes.h>

#include <cmath>
#include <vector>

namespace {

template <typename T>
uint8_t MapTo256(T l, T inMin, T inMax) {
  uint8_t outMin = 0;
  uint8_t outMax = 255;
  if (inMax - inMin + outMin == 0) return 0;

  return ((l - inMin) * (outMax - outMin)) / (inMax - inMin) + outMin;
}

}  // namespace

LEDController::LEDController() { SetCurrentPattern(ambient_patterns[0], 0); }

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

void LEDController::InitLEDs(int sizeX, int sizeY) {
  InitLedBuffer(sizeX * sizeY);
  float centerX = sizeX / 2;
  float centerY = sizeY / 2;
  leds_.reserve(led_buffer_.size());

  float max_radius = std::max(centerX, centerY);
  for (int y = 0; y < sizeY; ++y) {
    bool even = ((y % 2) == 0);
    for (int x = 0; x < sizeX; ++x) {
      int corrected_x = even ? x : sizeX - x - 1;
      uint8_t x256 = MapTo256(corrected_x, 0, sizeX);
      uint8_t y256 = MapTo256(y, 0, sizeY);
      float angle = atan2(centerY - y, centerX - x);
      uint8_t angle256 = MapTo256(angle, 0.f, (float)(2 * PI));
      float radius = hypot(x - centerX, y - centerY);
      uint8_t radius256 = MapTo256(radius, 0.f, max_radius);
      leds_.push_back(
          LED(led_buffer_[(y * sizeY) + x], x256, y256, angle256, radius256));
    }
  }
}

void LEDController::SetCurrentAnimation(WallAnimation animation) {
  if (current_animation_ == animation) return;
  current_animation_ = animation;
  switch (current_animation_) {
    case WallAnimation::kAmbient: {
      SetCurrentPattern(ambient_patterns[current_ambient_pattern_]);
      break;
    }
    case WallAnimation::kTouched: {
      break;
    }
  }
}

void LEDController::InitLedBuffer(int num_leds) {
  led_buffer_.assign(num_leds, CRGB::Black);
}

void LEDController::SetCurrentPattern(AmbientPattern pattern,
                                      int transition_duration_millis) {
  if (current_pattern_ == pattern) return;
  previous_pattern_ = current_pattern_;
  current_pattern_ = pattern;
  transition_start_millis_ = millis();
  transition_duration_millis_ = transition_duration_millis;
}

void LEDController::Update() {
  switch (current_animation_) {
    case WallAnimation::kAmbient: {
      // Ambient switches the pattern every N seconds.
      EVERY_N_SECONDS(5) {
        current_ambient_pattern_ =
            (current_ambient_pattern_ + 1) % ambient_patterns.size();
        SetCurrentPattern(ambient_patterns[current_ambient_pattern_], 1000);
      }
      break;
    }
    default:
      break;
  }

  // Calculate how much to blend the current pattern with the previous
  // pattern.
  uint64_t now = millis();
  uint64_t elapsed = now - transition_start_millis_;
  fract8 blend = 255;
  if (elapsed < transition_duration_millis_) {
    float ratio = float(elapsed) / float(transition_duration_millis_);
    blend = ratio * 255;
    blend = ease8InOutCubic(blend);
  }

  fill_solid(led_buffer_.data(), led_buffer_.size(), CRGB::Black);
  current_pattern_(leds_, blend);
  // Optimization: don't bother calling previous animation if blend_factor is
  // 255.
  if (blend < 255) {
    // Call the current pattern's function.
    if (previous_pattern_ != nullptr) previous_pattern_(leds_, 255 - blend);
  }
}

void LEDController::OutwardWave(std::vector<LED>& leds, fract8 blend_factor) {
  uint8_t wave_phase = beat8(20);

  for (LED& led : leds) {
    uint8_t brightness = sin8(mul8(led.radius() - wave_phase, 2));
    // Don't set the pixel directly: just add to it.
    CRGB color = CRGB(128, 0, 128).fadeToBlackBy(255 - brightness);
    nblend(led.color(), color, blend_factor);
  }
}

void LEDController::InwardWave(std::vector<LED>& leds, fract8 blend_factor) {
  uint8_t wave_phase = beat8(20);

  for (LED& led : leds) {
    uint8_t brightness = sin8(mul8(255 - led.radius() - wave_phase, 2));
    // Don't set the pixel directly: just add to it.
    CRGB color = CRGB(128, 0, 128).fadeToBlackBy(255 - brightness);
    nblend(led.color(), color, blend_factor);
  }
}

void LEDController::RainbowHorizontal(std::vector<LED>& leds,
                                      fract8 blend_factor) {
  uint8_t offset = beat8(20);
  for (LED& led : leds) {
    nblend(led.color(), ColorFromPalette(RainbowColors_p, led.y() + offset),
           blend_factor);
  }
}

void LEDController::RainbowVertical(std::vector<LED>& leds,
                                    fract8 blend_factor) {
  uint8_t offset = beat8(20);
  for (LED& led : leds) {
    nblend(led.color(), ColorFromPalette(RainbowColors_p, led.x() + offset),
           blend_factor);
  }
}
