#ifndef INCLUDE_MASTER_WALL_H_
#define INCLUDE_MASTER_WALL_H_

#include <ArduinoJson.hpp>
#include <cstdint>

#include "common/common.h"
#include "common/messages.h"

enum class DeliveryStatus {
  kUnknown,
  kSuccess,
  kFailure,
};

// The Wall class tracks the state of a given wall. Each wall can communicate to
// the wall MCU in order to change the currently playing animation.
class Wall {
 public:
  // Init with the MAC address of the wall.
  explicit Wall(MacAddress address);

  // Connect to the wall.
  void Connect();

  // MAC address of the wall.
  const MacAddress& address() const { return address_; }

  bool pressed() const { return pressed_; }

  DeliveryStatus last_delivery_status() const { return last_delivery_status_; }
  void set_last_delivery_status(DeliveryStatus status) {
    last_delivery_status_ = status;
  }

  // Last time a wall was pressed or released.
  uint64_t last_interaction_time_millis() const {
    return last_interaction_time_millis_;
  }

  // Handler for the hand pressed signal coming from the wall MCU.
  void OnHandPressed();

  // Handler for the hand released signal coming from the wall MCU.
  void OnHandReleased();

  void SetPattern(PatternId pattern_id, uint8_t pattern_speed,
                  int transition_duration_millis);

  // Send a command to the wall MCU.
  void SendSetPatternCommand(const SetPatternCommand& command) const;
  void SendRestartCommand() const;

 private:
  void Send(const ArduinoJson::JsonDocument& doc) const;
  DeliveryStatus last_delivery_status_;

  // MAC address of the wall being controlled.
  MacAddress address_;

  bool pressed_;
  // If hand is pressed, time at which it became pressed.
  uint64_t last_interaction_time_millis_;
};

#endif  // INCLUDE_MASTER_WALL_H_
