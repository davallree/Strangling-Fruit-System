#ifndef INCLUDE_MASTER_SERIAL_H_
#define INCLUDE_MASTER_SERIAL_H_

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <mutex>

namespace serial {
// Method names.
inline constexpr char kDebugMethod[] = "debug";
inline constexpr char kPlaySoundMethod[] = "playSound";
inline constexpr char kPlayOneShotMethod[] = "playOneShot";

// Parameters.
inline constexpr char kSoundNameParam[] = "soundName";
inline constexpr char kSoundParamsParam[] = "soundParams";
inline constexpr char kPressedCountParam[] = "pressedCount";

// Send a debug message to the PC.
void Debug(const char* format, ...);

// Play the ambient sound.
void PlayAmbientSound();

// Play the glitch sound.
void PlayGlitchSound();

// Play the climax sound.
void PlayClimaxSound();

// Play the pressed sound for the given number of pressed hands.
void PlayPressedSound(uint8_t pressed_count);

// Play the dull sound when hands are disabled.
void PlayDullSound();
}  // namespace serial

#endif  // INCLUDE_MASTER_SERIAL_H_
