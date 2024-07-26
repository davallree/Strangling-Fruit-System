#include "master/cube.h"

#include <Arduino.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "master/wall.h"

namespace {

bool WallsAllPressed(const std::vector<Wall>& walls) {
  for (const Wall& wall : walls) {
    if (wall.state() != WallState::kPressed) return false;
  }
  return true;
}

std::optional<uint64_t> LatestPressedTime(const std::vector<Wall>& walls) {
  std::optional<uint64_t> latest_pressed_time;
  for (const Wall& wall : walls) {
    if (wall.state() == WallState::kPressed) {
      if (!latest_pressed_time ||
          *latest_pressed_time < wall.pressed_start_millis()) {
        latest_pressed_time = wall.pressed_start_millis();
      }
    }
  }
  return latest_pressed_time;
}

bool ShouldGlitch(const std::vector<Wall>& walls) {
  std::optional<uint64_t> latest_wall_press_time = LatestPressedTime(walls);
  if (latest_wall_press_time) {
    uint64_t pressed_duration_millis = millis() - *latest_wall_press_time;
    if (pressed_duration_millis > Cube::kGlitchTimeoutMillis) {
      return true;
    }
  }
  return false;
}

}  // namespace

Cube& Cube::AddWall(Wall wall) {
  walls_.push_back(std::move(wall));
  return *this;
}

void Cube::Connect() {
  for (Wall& wall : walls_) {
    wall.Connect();
  }
}

void Cube::Update() {
  // Update all the walls.
  for (Wall& wall : walls_) wall.Update();

  switch (state_) {
    case CubeState::kDefault: {
      // Check if we need to enter climax state.
      if (WallsAllPressed(walls_)) {
        Serial.println("Entering climax state.");
        // Set the cube and all the walls to climax state.
        SetState(CubeState::kClimax);
        for (Wall& wall : walls_) {
          wall.SetState(WallState::kClimax);
        }
      } else if (ShouldGlitch(walls_)) {
        Serial.println("Timed out, entering glitch state.");
        SetState(CubeState::kGlitched);
        for (Wall& wall : walls_) {
          wall.SetState(WallState::kGlitched);
        }
      }
      break;
    }
    case CubeState::kGlitched: {
      // Check if we need to exit glitched state.
      uint64_t time_in_glitched_state_millis = millis() - state_entered_millis_;
      if (time_in_glitched_state_millis > kGlitchDurationMillis) {
        SetState(CubeState::kDefault);
        for (Wall& wall : walls_) {
          wall.SetState(WallState::kUnpressed);
        }
      }
      break;
    }
    case CubeState::kClimax: {
      // Check if we need to exit climax state.
    }
  }
}

Wall* Cube::GetWall(MacAddress address) {
  for (Wall& wall : walls_) {
    if (wall.address() == address) return &wall;
  }
  return nullptr;
}

void Cube::OnHandEvent(const MacAddress& mac_address,
                       const HandEvent& hand_event) {
  // Get the wall that sent the event.
  Wall* wall = GetWall(mac_address);
  if (wall == nullptr) {
    Serial.println("Unknown wall.");
  }

  // Set the state for that wall.
  if (hand_event.type == HandEventType::kPressed) {
    wall->OnHandPressed();
  }
  if (hand_event.type == HandEventType::kReleased) {
    wall->OnHandReleased();
  }
}

void Cube::SetState(CubeState state) {
  state_ = state;
  state_entered_millis_ = millis();
}
