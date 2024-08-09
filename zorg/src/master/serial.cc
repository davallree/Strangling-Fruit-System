#include "master/serial.h"

#include "common/messages.h"
#include "master/cube.h"

namespace serial {
namespace {

void SendJson(const ArduinoJson::JsonDocument& doc) {
  static std::mutex serial_mutex;
  std::lock_guard<std::mutex> lock(serial_mutex);
  ArduinoJson::serializeJson(doc, Serial);
  Serial.println();
}

}  // namespace

void Debug(const char* format, ...) {
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

void PlayAmbientSound() {
  ArduinoJson::JsonDocument msg;
  msg[kMethod] = kPlaySoundMethod;
  msg[kParams][kSoundNameParam] = "ambient";
  SendJson(msg);
}

void PlayGlitchSound() {
  ArduinoJson::JsonDocument msg;
  msg[kMethod] = kPlaySoundMethod;
  msg[kParams][kSoundNameParam] = "glitch";
  SendJson(msg);
}

void PlayClimaxSound() {
  ArduinoJson::JsonDocument msg;
  msg[kMethod] = kPlaySoundMethod;
  msg[kParams][kSoundNameParam] = "climax";
  SendJson(msg);
}

void PlayPressedSound(uint8_t pressed_count) {
  ArduinoJson::JsonDocument msg;
  msg[kMethod] = kPlaySoundMethod;
  msg[kParams][kSoundNameParam] = "pressed";
  msg[kParams][kSoundParamsParam][kPressedCountParam] = pressed_count;
  SendJson(msg);
}

void PlayDullSound() {
  ArduinoJson::JsonDocument msg;
  msg[kMethod] = kPlayOneShotMethod;
  msg[kParams][kSoundNameParam] = "dull";
  SendJson(msg);
}

void UpdateStatus(const Cube& cube) {
  ArduinoJson::JsonDocument msg;
  msg[kMethod] = "updateStatus";
  for (const Wall& wall : cube.walls()) {
    ArduinoJson::JsonDocument wall_status;
    wall_status["address"] = MacAddressToString(wall.address());
    switch (wall.last_delivery_status()) {
      case DeliveryStatus::kSuccess:
        wall_status["lastDeliveryStatus"] = "success";
        break;
      case DeliveryStatus::kFailure:
        wall_status["lastDeliveryStatus"] = "failure";
        break;
      default:
        wall_status["lastDeliveryStatus"] = "unknown";
    }
    msg[kParams]["walls"].add(wall_status);
  }
  SendJson(msg);
}

}  // namespace serial
