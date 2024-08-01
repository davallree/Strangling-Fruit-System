#ifndef INCLUDE_COMMON_MESSAGES_H_
#define INCLUDE_COMMON_MESSAGES_H_

#include <cstdint>

// The type of hand event.
enum class HandEventType : uint8_t {
  // Sent when the hand has been pressed.
  kPressed,
  // Sent when the hand was released.
  kReleased
};

// A hand event, sent from the wall controllers to the master controller.
struct HandEvent {
  // Whether the hand was pressed or released.
  HandEventType type;
};

enum PatternId : uint8_t {
  kNone,

  // Ambient patterns start.
  kSpiral,
  kOutWave,
  kRose,
  kCircles,
  // Ambient patterns end.

  kInWave,
  kGlitch,
  kNumPatternIds,
};

struct SetPatternCommand {
  PatternId pattern_id;
  uint8_t pattern_speed;
  int transition_duration_millis;
};

#endif  // INCLUDE_COMMON_MESSAGES_H_
