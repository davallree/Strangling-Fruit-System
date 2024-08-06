#include "master/cube.h"

#include <Arduino.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "master/serial.h"
#include "master/wall.h"

namespace {

// Check if all the walls are currently pressed.
bool WallsAllPressed(const std::vector<Wall>& walls) {
  for (const Wall& wall : walls) {
    if (!wall.pressed()) return false;
  }
  return true;
}

bool NoWallsPressed(const std::vector<Wall>& walls) {
  for (const Wall& wall : walls) {
    if (wall.pressed()) return false;
  }
  return true;
}

float WallPressedCount(const std::vector<Wall>& walls) {
  int pressed_count = 0;
  for (const Wall& wall : walls) {
    if (wall.pressed()) pressed_count++;
  }
  return pressed_count;
}

// For all the currently pressed hands, returns the time the last one to be
// pressed. If no hands are currently pressed, returns nullopt;
uint64_t LatestInteractionTime(const std::vector<Wall>& walls) {
  uint64_t latest_interaction_time_ = 0;
  for (const Wall& wall : walls) {
    if (wall.last_interaction_time_millis() > latest_interaction_time_)
      latest_interaction_time_ = wall.last_interaction_time_millis();
  }
  return latest_interaction_time_;
}

// Check if the cube should glitch.
bool ShouldGlitch(const std::vector<Wall>& walls) {
  uint64_t latest_interaction_time = LatestInteractionTime(walls);
  uint64_t elapsed = millis() - latest_interaction_time;
  return elapsed > Cube::kGlitchTimeoutMillis;
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
  SetState(CubeState::kAmbient);
}

void Cube::Update() {
  switch (state_) {
    case CubeState::kAmbient: {
      // Cycle through ambient patterns.
      if (millis() > next_pattern_time_) {
        PatternId first_pattern_id = PatternId::kSpiral;
        PatternId last_pattern_id = PatternId::kCircles;
        current_ambient_pattern_ =
            static_cast<PatternId>(current_ambient_pattern_ + 1);
        if (current_ambient_pattern_ > last_pattern_id)
          current_ambient_pattern_ = first_pattern_id;
        next_pattern_time_ = millis() + kAmbientCycleMillis;
        for (Wall& wall : walls_) {
          wall.SetPattern(current_ambient_pattern_, kAmbientSpeed,
                          kAmbientTransitionMillis);
        }
      }
      break;
    }
    case CubeState::kTouched: {
      if (ShouldGlitch(walls_)) {
        serial::Debug("Timed out, entering glitch state.");
        SetState(CubeState::kGlitched);
      }
      break;
    }
    case CubeState::kGlitched: {
      // Check if we need to exit glitched state.
      uint64_t time_in_glitched_state_millis = millis() - state_entered_millis_;
      if (time_in_glitched_state_millis > kGlitchDurationMillis) {
        serial::Debug("Leaving glitched state.");
        SetState(CubeState::kAmbient);
      }
      break;
    }
    case CubeState::kClimax: {
      // Check if we need to exit climax state.
      uint64_t time_in_climax_state_millis = millis() - state_entered_millis_;
      if (time_in_climax_state_millis > kClimaxDurationMillis) {
        serial::Debug("Leaving climax state.");
        SetState(CubeState::kAmbient);
      }
      break;
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
    serial::Debug("Unknown wall.");
  }

  // Set the state for that wall and update its pattern.
  // TODO: use the number of pressed/unpressed walls to influence the patterns
  // playing.
  if (hand_event.type == HandEventType::kPressed) {
    wall->OnHandPressed();
  }
  if (hand_event.type == HandEventType::kReleased) {
    wall->OnHandReleased();
  }

  if (state_ == CubeState::kGlitched || state_ == CubeState::kClimax) {
    serial::Debug("Cube is glitched/climaxed, ignoring hand.");
    return;
  }

  // Check if we need to enter climax state.
  if (WallsAllPressed(walls_)) {
    serial::Debug("Entering climax state.");
    // Set the cube and all the walls to climax state.
    SetState(CubeState::kClimax);
    return;
  }

  if (NoWallsPressed(walls_)) {
    SetState(CubeState::kAmbient);
    return;
  }
  SetState(CubeState::kTouched);

  // Check how many walls are pressed, and set the patterns accordingly.
  int num_walls_pressed = WallPressedCount(walls_);
  float pressed_ratio = (float)num_walls_pressed / (float)walls_.size();
  uint8_t min_speed = 60;
  uint8_t max_speed = 180;
  uint8_t diff = max_speed - min_speed;
  uint8_t speed = min_speed + (pressed_ratio * diff);
  for (Wall& wall : walls_) {
    if (wall.pressed()) {
      wall.SetPattern(PatternId::kInWave, speed, 200);
    } else {
      wall.SetPattern(PatternId::kOutWave, speed, 200);
    }
  }
  // Play the pressed sound.
  serial::PlayPressedSound(num_walls_pressed);
}

void Cube::SetState(CubeState state) {
  if (state_ == state) return;
  state_ = state;
  state_entered_millis_ = millis();
  switch (state) {
    case CubeState::kAmbient: {
      // When entering the default state, set all the walls to the current
      // pattern, and set the next pattern time.
      for (Wall& wall : walls_) {
        wall.SetPattern(current_ambient_pattern_, kAmbientSpeed,
                        kAmbientTransitionMillis);
      }
      next_pattern_time_ = millis() + kAmbientCycleMillis;
      // Start playing the ambient sound.
      serial::PlayAmbientSound();
      break;
    }
    case CubeState::kTouched: {
      break;
    }
    case CubeState::kGlitched: {
      for (Wall& wall : walls_) {
        wall.SetPattern(PatternId::kGlitch, kGlitchSpeed, 0);
      }
      serial::PlayGlitchSound();
      break;
    }
    case CubeState::kClimax: {
      for (Wall& wall : walls_) {
        wall.SetPattern(PatternId::kClimaxPhaseOne, 80, 200);
      }
      break;
    }
  }
}
