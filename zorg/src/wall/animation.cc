#include "wall/animation.h"

#include <pixeltypes.h>

#include <vector>

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
