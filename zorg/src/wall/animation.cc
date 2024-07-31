#include "wall/animation.h"

#include <pixeltypes.h>

#include <cmath>
#include <vector>

LEDController::LEDController() { SetCurrentPattern(ambient_patterns[0], 0); }

void LEDController::InitLEDs(int num_leds, const uint8_t* coordsX,
                             const uint8_t* coordsY, const uint8_t* angles,
                             const uint8_t* radii) {
  InitBuffers(num_leds);
  for (int i = 0; i < num_leds; ++i) {
    led_buffer_.AddLED(coordsX[i], coordsY[i], angles[i], radii[i]);
    previous_buffer_.AddLED(coordsX[i], coordsY[i], angles[i], radii[i]);
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
      // Transition more quickly into touched.
      SetCurrentPattern(TouchedPattern, 200);
      break;
    }
    case WallAnimation::kGlitch: {
      // Glitch pattern kicks in without transition.
      SetCurrentPattern(GlitchedPattern, 0);
    }
  }
}

void LEDController::InitBuffers(int num_leds) {
  led_buffer_.Init(num_leds);
  previous_buffer_.Init(num_leds);
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
  uint64_t now = millis();
  uint64_t elapsed = now - transition_start_millis_;
  bool in_transition = elapsed < transition_duration_millis_;
  switch (current_animation_) {
    case WallAnimation::kAmbient: {
      // Ambient switches the pattern every N seconds, unless there's an ongoing
      // transition.
      EVERY_N_SECONDS(kAmbientPatternDurationSeconds) {
        if (!in_transition) {
          current_ambient_pattern_ =
              (current_ambient_pattern_ + 1) % ambient_patterns.size();
          SetCurrentPattern(ambient_patterns[current_ambient_pattern_], 1000);
          return;
        }
      }
      break;
    }
    default:
      break;
  }
  // Call the current pattern.
  current_pattern_(led_buffer_);

  // Calculate how much to blend the current pattern with the previous
  // pattern.
  if (in_transition) {
    float ratio = float(elapsed) / float(transition_duration_millis_);
    fract8 blend = ratio * 255;
    blend = ease8InOutCubic(blend);
    // Fade in the current pattern.
    fadeToBlackBy(led_buffer_.raw_led_data(), led_buffer_.num_leds(),
                  255 - blend);
    // Call the previous pattern's function, but store in the alternate buffer.
    if (previous_pattern_ != nullptr) {
      previous_pattern_(previous_buffer_);
      // Fade out the previous pattern.
      // fadeToBlackBy(previous_buffer_.raw_led_data(),
      //               previous_buffer_.num_leds(), blend);
      // Blend the two.
      nblend(led_buffer_.raw_led_data(), previous_buffer_.raw_led_data(),
             led_buffer_.num_leds(), 255 - blend);
    }
  }
}

void LEDController::OutwardWave(LEDBuffer& buffer) {
  uint8_t wave_phase = beat8(20);

  for (LED& led : buffer.leds()) {
    uint8_t brightness = sin8(mul8(led.radius() - wave_phase, 2));
    // Don't set the pixel directly: just add to it.
    led.color().setRGB(128, 0, 128).fadeToBlackBy(255 - brightness);
  }
}

void LEDController::InwardWave(LEDBuffer& buffer) {
  uint8_t wave_phase = beat8(20);

  for (LED& led : buffer.leds()) {
    uint8_t brightness = sin8(mul8(255 - led.radius() - wave_phase, 2));
    led.color().setRGB(128, 0, 128).fadeToBlackBy(255 - brightness);
  }
}

void LEDController::RainbowHorizontal(LEDBuffer& buffer) {
  uint8_t offset = beat8(20);
  for (LED& led : buffer.leds()) {
    led.color() = ColorFromPalette(RainbowColors_p, led.y() + offset);
  }
}

void LEDController::RainbowVertical(LEDBuffer& buffer) {
  uint8_t offset = beat8(20);
  for (LED& led : buffer.leds()) {
    led.color() = ColorFromPalette(RainbowColors_p, led.x() + offset);
  }
}

void LEDController::TouchedPattern(LEDBuffer& buffer) {
  uint8_t wave_phase = beat8(60);

  for (LED& led : buffer.leds()) {
    uint8_t brightness = sin8(mul8(255 - led.radius() - wave_phase, 2));
    led.color().setRGB(128, 0, 128).fadeToBlackBy(255 - brightness);
  }
}

void LEDController::Spiral(LEDBuffer& buffer) {
  uint8_t twist = 2;
  uint8_t strands = 4;

  for (LED& led : buffer.leds()) {
    uint8_t brightness =
        sin8(twist * led.radius() + strands * led.angle() + beat8(60));
    // uint8_t brightness = sin8(255 - led.radius() - wave_phase);
    led.color().setHSV(212, 255, brightness);
  }
}

void LEDController::Rose(LEDBuffer& buffer) {
  uint8_t zoom = 1;
  uint8_t shape = 1;
  uint8_t petals = 3;
  uint8_t rotation = beat8(60);
  uint8_t ripple = beat8(10);

  for (LED& led : buffer.leds()) {
    uint8_t brightness =
        sin8(zoom * led.radius() +
             shape * sin8(petals * led.angle() + rotation) + ripple);
    led.color().setHSV(212, 255, brightness);
  }
}

void LEDController::GlitchedPattern(LEDBuffer& buffer) {
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
