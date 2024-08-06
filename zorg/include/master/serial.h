#ifndef INCLUDE_MASTER_SERIAL_H_
#define INCLUDE_MASTER_SERIAL_H_

#include <Arduino.h>

#include <ArduinoJson.hpp>

namespace serial {
inline constexpr char kMethod[] = "method";
inline constexpr char kParams[] = "params";
inline constexpr int TIMEOUT_MS = 1000;

inline void SendJson(const ArduinoJson::JsonDocument& doc) {
  ArduinoJson::serializeJson(doc, Serial);
  Serial.println();
  // Wait for the ack, with a timeout.
  unsigned long start_time = millis();
  while (!Serial.available()) {
    if ((millis() - start_time) > TIMEOUT_MS) {
      // Timeout occurred, handle the error.
      break;
    }
  }
  // Read the ack, if any.
  if (Serial.available()) Serial.read();
}

inline constexpr char kDebugMethod[] = "debug";
inline void Debug(const char* format, ...) {
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  ArduinoJson::JsonDocument msg;
  msg[kMethod] = kDebugMethod;
  msg[kParams][0] = buffer;
  SendJson(msg);
}

inline constexpr char kPlaySoundMethod[] = "playSound";
inline constexpr char kSoundNameParam[] = "soundName";
inline void PlayAmbientSound() {
  ArduinoJson::JsonDocument msg;
  msg[kMethod] = kPlaySoundMethod;
  msg[kParams][kSoundNameParam] = "ambient";
  SendJson(msg);
}

inline void PlayGlitchSound() {
  ArduinoJson::JsonDocument msg;
  msg[kMethod] = kPlaySoundMethod;
  msg[kParams][kSoundNameParam] = "glitch";
  SendJson(msg);
}

inline constexpr char kSoundParamsParam[] = "soundParams";
inline constexpr char kPressedCountParam[] = "pressedCount";
inline void PlayPressedSound(uint8_t pressed_count) {
  ArduinoJson::JsonDocument msg;
  msg[kMethod] = kPlaySoundMethod;
  msg[kParams][kSoundNameParam] = "pressed";
  msg[kParams][kSoundParamsParam][kPressedCountParam] = pressed_count;
  SendJson(msg);
}

}  // namespace serial

#endif  // INCLUDE_MASTER_SERIAL_H_
