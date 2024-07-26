// Master controller.
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

#include <array>
#include <vector>

#include "common/common.h"
#include "common/messages.h"

enum class WallState : uint8_t {
  kUnpressed,
  kPressed,
  kGlitched,
};
class Wall {
 public:
  static const uint64_t kGlitchTimeoutMillis = 5000;
  static const uint64_t kGlitchDurationMillis = 10000;

  explicit Wall(MacAddress address) : address_(std::move(address)) {}

  void SendCommand(const MasterCommand& command) const {
    Serial.println("Sending command to wall.");
    esp_err_t result = esp_now_send(address_.data(),
                                    reinterpret_cast<const uint8_t*>(&command),
                                    sizeof(command));
    if (result == ESP_OK) {
      Serial.println("Message sent successfully");
    } else {
      Serial.println("Error sending the message");
    }
  }

  const MacAddress& address() const { return address_; }

  WallState state() const { return state_; }

  void OnHandPressed() {
    if (state_ == WallState::kGlitched) {
      Serial.println("Wall is glitched, ignoring press.");
      return;
    }
    SetState(WallState::kPressed);
  }

  void OnHandReleased() {
    if (state_ == WallState::kGlitched) {
      Serial.println("Wall is glitched, ignoring release.");
      return;
    }
    SetState(WallState::kUnpressed);
  }

  void SetState(WallState state) {
    Serial.println("Setting state.");
    // Don't do a state transition for the same state.
    if (state_ == state) return;
    state_ = state;
    MasterCommand command;
    switch (state_) {
      case WallState::kUnpressed:
        command.animation_to_play = WallAnimation::kAmbient;
        break;
      case WallState::kPressed: {
        pressed_start_millis_ = millis();
        command.animation_to_play = WallAnimation::kTouched;
        break;
      }
      case WallState::kGlitched: {
        glitched_end_millis_ = millis() + kGlitchDurationMillis;
        command.animation_to_play = WallAnimation::kGlitch;
        break;
      }
    }
    SendCommand(command);
  }

  void Update() {
    // Check for glitch.
    if (state_ == WallState::kPressed) {
      uint64_t pressed_duration_millis = millis() - pressed_start_millis_;
      if (pressed_duration_millis > kGlitchTimeoutMillis) {
        Serial.println("timed out, entering glitch.");
        SetState(WallState::kGlitched);
      }
    }
    if (state_ == WallState::kGlitched) {
      // Check if we need to exit glitched state.
      if (millis() > glitched_end_millis_) {
        Serial.println("glitch ended.");
        SetState(WallState::kUnpressed);
      }
    }
    // TODO: add a low-frenquency call to SendCommand, in case a message got
    // dropped.
  }

  uint64_t pressed_start_millis() const { return pressed_start_millis_; }

 private:
  MacAddress address_;
  WallState state_ = WallState::kUnpressed;
  uint64_t pressed_start_millis_;
  uint64_t glitched_end_millis_;
};

std::vector<Wall> walls;
int GetWallIndex(MacAddress address) {
  for (int i = 0; i < walls.size(); i++) {
    if (walls[i].address() == address) return i;
  }
  return -1;
}

void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  // Debug outgoing data.
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success"
                                                : "Delivery Fail");
}

void OnDataReceived(const uint8_t* raw_addr, const uint8_t* data,
                    int data_len) {
  Serial.print("\r\nLast Packet Received:\t");
  for (int i = 0; i < data_len; i++) {
    Serial.printf("%#x", data[i]);
  }
  Serial.println();

  // Get the wall using the address.
  MacAddress address = MacAddressFromArray(raw_addr);
  int wall_index = GetWallIndex(address);
  if (wall_index < 0) {
    Serial.println("Unknown wall.");
  }
  Wall& wall = walls[wall_index];

  // Parse the event from the wall.
  HandEvent event;
  memcpy(&event, data, sizeof(event));

  if (event.type == HandEventType::kPressed) {
    wall.OnHandPressed();
  }
  if (event.type == HandEventType::kReleased) {
    wall.OnHandReleased();
  }
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  // Master always has the LED turned on.
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  InitEspNow();
  Serial.println("Master MAC address: " + WiFi.macAddress());

  esp_now_register_send_cb(&OnDataSent);
  esp_now_register_recv_cb(&OnDataReceived);

  walls.push_back(Wall({0xE8, 0x9F, 0x6D, 0x1F, 0x84, 0x44}));

  for (Wall& wall : walls) {
    AddPeer(wall.address());
    wall.SendCommand(
        MasterCommand{.animation_to_play = WallAnimation::kAmbient});
  }
}

void loop() {
  // Check walls for glitch.
  for (Wall& wall : walls) {
    wall.Update();
  }
}
