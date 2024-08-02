#ifndef INCLUDE_MASTER_SERIAL_H_
#define INCLUDE_MASTER_SERIAL_H_

#include <ArduinoJson.hpp>

namespace serial {
inline constexpr char kMethod[] = "method";
inline constexpr char kParams[] = "params";

inline constexpr char kDebugMethod[] = "debug";
inline void debug(const String& s) {
  ArduinoJson::JsonDocument msg;
  msg[kMethod] = kDebugMethod;
  msg[kParams][0] = s;
  ArduinoJson::serializeJson(msg, Serial);
  Serial.println();
}

inline constexpr char kPlaySoundMethod[] = "playSound";
inline constexpr char kSoundNameParam[] = "soundName";
inline void PlayAmbientSound() {
  ArduinoJson::JsonDocument msg;
  msg[kMethod] = kPlaySoundMethod;
  msg[kParams][kSoundNameParam] = "ambient";
  ArduinoJson::serializeJson(msg, Serial);
  Serial.println();
}

inline constexpr char kSoundParamsParam[] = "soundParams";
inline constexpr char kPressedCountParam[] = "pressedCount";
inline void PlayPressedSound(uint8_t pressed_count) {
  ArduinoJson::JsonDocument msg;
  msg[kMethod] = kPlaySoundMethod;
  msg[kParams][kSoundNameParam] = "pressed";
  msg[kParams][kSoundParamsParam][kPressedCountParam] = pressed_count;
  ArduinoJson::serializeJson(msg, Serial);
  Serial.println();
}

}  // namespace serial

#endif  // INCLUDE_MASTER_SERIAL_H_
