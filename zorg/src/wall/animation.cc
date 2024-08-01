#include "wall/animation.h"

#include <pixeltypes.h>

#include <cmath>
#include <vector>

LEDController::LEDController() {
  patterns_.resize(PatternId::kNumPatternIds);
  patterns_[PatternId::kNone] = std::make_unique<NonePattern>();
  patterns_[PatternId::kSpiral] = std::make_unique<SpiralPattern>();
  patterns_[PatternId::kRose] = std::make_unique<RosePattern>();
  patterns_[PatternId::kCircles] = std::make_unique<CirclesPattern>();
  patterns_[PatternId::kInWave] =
      std::make_unique<WavePattern>(WavePattern::Direction::kIn);
  patterns_[PatternId::kOutWave] =
      std::make_unique<WavePattern>(WavePattern::Direction::kOut);
  patterns_[PatternId::kGlitch] = std::make_unique<GlitchPattern>();
}

void LEDController::InitLEDs(int num_leds, const std::vector<uint8_t> coordsX,
                             const std::vector<uint8_t> coordsY,
                             const std::vector<uint8_t> angles,
                             const std::vector<uint8_t> radii) {
  InitBuffers(num_leds);
  for (int i = 0; i < num_leds; ++i) {
    led_buffer_.AddLED(coordsX[i], coordsY[i], angles[i], radii[i]);
    previous_buffer_.AddLED(coordsX[i], coordsY[i], angles[i], radii[i]);
  }
}

void LEDController::InitBuffers(int num_leds) {
  led_buffer_.Init(num_leds);
  previous_buffer_.Init(num_leds);
}

void LEDController::SetCurrentPattern(PatternId pattern_id,
                                      uint8_t pattern_speed,
                                      int transition_duration_millis) {
  if (current_pattern_id_ == pattern_id) {
    // Same pattern, just update the speed.
    current_pattern_speed_ = pattern_speed;
  } else {
    previous_pattern_id_ = current_pattern_id_;
    previous_pattern_speed_ = current_pattern_speed_;
    current_pattern_id_ = pattern_id;
    current_pattern_speed_ = pattern_speed;
    transition_start_millis_ = millis();
    transition_duration_millis_ = transition_duration_millis;
  }
}

void LEDController::Update() {
  // Call the current pattern.
  Pattern* current_pattern = patterns_[current_pattern_id_].get();
  if (current_pattern == nullptr) {
    EVERY_N_SECONDS(1) {
      Serial.printf("No pattern registered for id=%d.\n", current_pattern_id_);
    }
    return;
  }
  current_pattern->Update(led_buffer_, current_pattern_speed_);

  // Calculate how much to blend the current pattern with the previous
  // pattern.
  uint64_t elapsed = millis() - transition_start_millis_;
  if (elapsed < transition_duration_millis_) {
    float ratio = float(elapsed) / float(transition_duration_millis_);
    fract8 blend = ratio * 255;
    blend = ease8InOutCubic(blend);
    // Fade in the current pattern.
    fadeToBlackBy(led_buffer_.raw_led_data(), led_buffer_.num_leds(),
                  255 - blend);
    // Call the previous pattern's function, but store in the alternate buffer.
    Pattern* previous_pattern = patterns_[previous_pattern_id_].get();
    if (previous_pattern == nullptr) {
      EVERY_N_SECONDS(1) {
        Serial.printf("No pattern registered for id=%d.\n",
                      previous_pattern_id_);
      }
      return;
    }
    previous_pattern->Update(previous_buffer_, previous_pattern_speed_);
    // Blend the two.
    nblend(led_buffer_.raw_led_data(), previous_buffer_.raw_led_data(),
           led_buffer_.num_leds(), 255 - blend);
  }
}

// void LEDController::TouchedPattern(LEDBuffer& buffer) {
//   uint8_t wave_phase = beat8(60);
//   uint8_t pulse = beatsin8(100, 0, 150);

//   for (LED& led : buffer.leds()) {
//     uint8_t brightness = sin8(mul8(255 - led.radius() - wave_phase, 2));
//     led.color()
//         .setRGB(128, 0, 128)
//         .fadeToBlackBy(255 - brightness)
//         .fadeToBlackBy(pulse);
//   }
// }

// void LEDController::Circles(LEDBuffer& buffer) {
//   uint8_t scale = 1;
//   uint8_t x_translation = 0;
//   uint8_t y_translation = 0;
//   uint8_t warp = beat8(60);

//   for (LED& led : buffer.leds()) {
//     uint8_t brightness = sin8(sin8(scale * led.x() + x_translation) +
//                               sin8(scale * led.y() + y_translation) + warp);
//     led.color().setHSV(212, 255, brightness);
//   }
// }

// void LEDController::Squiggles(LEDBuffer& buffer) {
//   uint8_t scale = 1;
//   uint8_t x_translation = 0;
//   uint8_t y_translation = 0;
//   uint8_t warp = beat8(60);

//   for (LED& led : buffer.leds()) {
//     uint8_t brightness = sin8(sin8(scale * led.x() + x_translation) +
//                               sin8(scale * led.y() + y_translation) +
//                               sin8(led.x() + led.y()) + warp);
//     led.color().setHSV(212, 255, brightness);
//   }
// }

// void LEDController::ClimaxPattern(LEDBuffer& buffer) {
//   uint8_t hue_offset = beat8(60);
//   uint8_t offset = beat8(200);
//   for (LED& led : buffer.leds()) {
//     led.color().setHSV(led.radius() + offset, 255,
//                        sin8(2 * led.radius() - offset));
//   }
// }
