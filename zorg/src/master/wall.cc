#include "master/wall.h"

#include <Arduino.h>
#include <esp_now.h>

#include <cstdint>

#include "common/common.h"
#include "common/messages.h"

Wall::Wall(MacAddress address) : address_(std::move(address)) {}

void Wall::Connect() {
  // Set up the connection to the wall.
  Serial.println("Connecting to wall.");
  AddPeer(address_);
  SetState(WallState::kUnpressed);
}

void Wall::SendCommand(const MasterCommand& command) const {
  Serial.println("Sending command to wall.");
  esp_err_t result =
      esp_now_send(address_.data(), reinterpret_cast<const uint8_t*>(&command),
                   sizeof(command));
  if (result == ESP_OK) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Error sending the message");
  }
}

void Wall::OnHandPressed() {
  if (state_ == WallState::kGlitched) {
    Serial.println("Wall is glitched, ignoring press.");
    return;
  }
  SetState(WallState::kPressed);
}

void Wall::OnHandReleased() {
  if (state_ == WallState::kGlitched) {
    Serial.println("Wall is glitched, ignoring release.");
    return;
  }
  SetState(WallState::kUnpressed);
}

void Wall::SetState(WallState state) {
  Serial.println("Setting state.");
  // Don't do a state transition for the same state.
  if (state_ == state) return;
  state_ = state;
  if (state_ == WallState::kPressed) {
    pressed_start_millis_ = millis();
  }
  dirty_ = true;
}

WallAnimation Wall::GetWallAnimation() {
  switch (state_) {
    case WallState::kUnpressed:
      return WallAnimation::kAmbient;
    case WallState::kPressed:
      return WallAnimation::kTouched;
    case WallState::kGlitched:
      return WallAnimation::kGlitch;
    case WallState::kClimax:
      return WallAnimation::kClimax;
  }
  // Fall through to idle.
  return WallAnimation::kAmbient;
}

void Wall::Update() {
  if (dirty_) {
    Serial.println("Wall dirty, updating.");
    MasterCommand command;
    command.animation_to_play = GetWallAnimation();
    SendCommand(command);
    dirty_ = false;
  }
  // TODO: add a low-frenquency call to SendCommand, in case a message got
  // dropped.
}
