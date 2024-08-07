#ifndef INCLUDE_MASTER_CUBE_H_
#define INCLUDE_MASTER_CUBE_H_

#include <cstdint>
#include <vector>

#include "master/wall.h"

enum class CubeState : uint8_t {
  kInvalid,
  kAmbient,
  kTouched,
  kGlitched,
  kRecovery,
  kClimax,
};

// This class manages the state of the cube.
class Cube {
 public:
  // How long each ambient pattern plays for.
  static constexpr int kAmbientCycleMillis = 60 * 1000;

  static constexpr int kAmbientTransitionMillis = 1 * 1000;
  static constexpr uint8_t kAmbientSpeed = 60;

  // How long until the cube enters glitch mode.
  static constexpr int kGlitchTimeoutMillis = 15 * 1000;
  // How long the cube stays in glitch mode.
  static constexpr int kGlitchDurationMillis = 10 * 1000;
  static constexpr uint8_t kGlitchSpeed = 60;

  // How long the cube stays in climax mode.
  static constexpr int kClimaxDurationMillis = 10 * 1000;

  // How long the cube stays in recovery mode.
  static constexpr int kRecoveryDurationMillis = 60 * 1000;

  // Registers a wall with the cube.
  Cube& AddWall(Wall wall);

  // Connects to all the wall MCUs.
  void Connect();

  // Call in loop() to update the state of the cube.
  void Update();

  // Returns the wall with the given MAC adddress, or nullptr if there is no
  // wall with that MAC address.
  Wall* GetWall(MacAddress address);

  // Process a hand event from the given MAC address.
  void OnHandEvent(const MacAddress& mac_address, const HandEvent& hand_event);

  const std::vector<Wall>& walls() { return walls_; }

 private:
  void SetState(CubeState state);

  std::vector<Wall> walls_;
  CubeState state_ = CubeState::kInvalid;
  uint64_t state_entered_millis_;

  // Ambient patterns.
  PatternId current_ambient_pattern_ = PatternId::kInWave;
  uint64_t next_pattern_time_ = 0;
};

#endif  // INCLUDE_MASTER_CUBE_H_
