#include "master/wall.h"

#include <Arduino.h>
#include <esp_now.h>

#include <cstdint>

#include "common/common.h"
#include "common/messages.h"
#include "master/serial.h"

Wall::Wall(MacAddress address) : address_(std::move(address)) {}

void Wall::Connect() {
  // Set up the connection to the wall.
  Serial.println("Connecting to wall.");
  AddPeer(address_);
}

void Wall::SetPattern(PatternId pattern_id, uint8_t pattern_speed,
                      int transition_duration_millis) {
  SendSetPatternCommand(SetPatternCommand{
      .pattern_id = pattern_id,
      .pattern_speed = pattern_speed,
      .transition_duration_millis = transition_duration_millis});
}

void Wall::SendSetPatternCommand(const SetPatternCommand& command) const {
  serial::Debug(
      "Sending SetPatternCommand{pattern_id=%d, pattern_speed=%d, "
      "transition_duration_millis=%d}.\n",
      command.pattern_id, command.pattern_speed,
      command.transition_duration_millis);
  esp_err_t result =
      esp_now_send(address_.data(), reinterpret_cast<const uint8_t*>(&command),
                   sizeof(command));
  if (result != ESP_OK) {
    serial::Debug("Error sending the message");
  }
}

void Wall::OnHandPressed() {
  pressed_ = true;
  last_interaction_time_millis_ = millis();
}

void Wall::OnHandReleased() {
  pressed_ = false;
  last_interaction_time_millis_ = millis();
}
