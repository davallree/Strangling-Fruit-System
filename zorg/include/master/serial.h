#ifndef INCLUDE_MASTER_SERIAL_H_
#define INCLUDE_MASTER_SERIAL_H_

#include <ArduinoJson.hpp>

namespace serial {
inline constexpr char kDebugMethod[] = "debug";

inline void debug(const String& s) {
  ArduinoJson::JsonDocument msg;
  msg["method"] = kDebugMethod;
  msg["params"][0] = s;
  ArduinoJson::serializeJson(msg, Serial);
  Serial.println();
}

}  // namespace serial

#endif  // INCLUDE_MASTER_SERIAL_H_
