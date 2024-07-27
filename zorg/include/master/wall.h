#ifndef INCLUDE_MASTER_WALL_H_
#define INCLUDE_MASTER_WALL_H_

#include <cstdint>

#include "common/common.h"
#include "common/messages.h"

// Current state of an individual wall.
enum class WallState : uint8_t {
  kUnpressed,
  kPressed,
  kGlitched,
  kClimax,
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

  // Current state of the wall.
  WallState state() const { return state_; }

  // If wall is in the kPressed state, time at which it started being pressed.
  uint64_t pressed_start_millis() const { return pressed_start_millis_; }

  // Handler for the hand pressed signal coming from the wall MCU.
  void OnHandPressed();

  // Handler for the hand released signal coming from the wall MCU.
  void OnHandReleased();

  // Set the current state of the wall.
  // TODO: replace with SetGlitched(bool).
  void SetState(WallState state);

  // Sends commands to the wall MCU if necessary
  void Update();

 private:
  // Send a command to the wall MCU.
  void SendCommand(const MasterCommand& command) const;

  WallAnimation GetWallAnimation();

  // MAC address of the wall being controlled.
  MacAddress address_;

  // Current state of the wall.
  WallState state_ = WallState::kUnpressed;

  // If state_ == WallState::kPressed, time at which it became pressed.
  uint64_t pressed_start_millis_;

  // Whether we need to send an update to the wall MCU.
  bool dirty_ = true;
};

#endif  // INCLUDE_MASTER_WALL_H_
