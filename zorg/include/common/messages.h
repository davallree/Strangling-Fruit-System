#ifndef INCLUDE_COMMON_MESSAGES_H_
#define INCLUDE_COMMON_MESSAGES_H_

#include <ArduinoJson.hpp>
#include <cstdint>

// Top level keys for the JSON messages.
inline constexpr char kMethod[] = "method";
inline constexpr char kParams[] = "params";

inline constexpr char kRestartMethod[] = "restart";

inline constexpr char kSetTouchThresholdMethod[] = "setTouchThreshold";
inline constexpr char kTouchThresholdParam[] = "touchThreshold";

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
  kInWave,
  kCircles,
  // Ambient patterns end.

  kAwaitTouch,
  kGlitch,

  // Climax stuff.
  kClimaxPhaseOne,

  kRecovery,
  kNumPatternIds,
};

struct SetPatternCommand {
  static constexpr char kMethodName[] = "setPattern";

  static SetPatternCommand FromJsonCommand(
      const ArduinoJson::JsonDocument& doc) {
    SetPatternCommand command;
    const auto& params = doc[kParams];
    command.pattern_id = params["patternId"];
    command.pattern_speed = params["patternSpeed"];
    command.transition_duration_millis = params["transitionDurationMillis"];
    return command;
  }

  ArduinoJson::JsonDocument ToJsonCommand() const {
    ArduinoJson::JsonDocument doc;
    doc[kMethod] = kMethodName;
    doc[kParams]["patternId"] = pattern_id;
    doc[kParams]["patternSpeed"] = pattern_speed;
    doc[kParams]["transitionDurationMillis"] = transition_duration_millis;
    return doc;
  }

  PatternId pattern_id;
  uint8_t pattern_speed;
  int transition_duration_millis;
};

#endif  // INCLUDE_COMMON_MESSAGES_H_
