#ifndef NEW_ZORG_COMMON_MESSAGES_H_
#define NEW_ZORG_COMMON_MESSAGES_H_
#ifndef INCLUDE_MESSAGES_H_
#define INCLUDE_MESSAGES_H_

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

enum class WallAnimation : uint8_t {
  kAmbient,
  kTouched,
  kGlitch,
};

struct MasterCommand {
  WallAnimation animation_to_play;
};

#endif  // INCLUDE_MESSAGES_H_


#endif // NEW_ZORG_COMMON_MESSAGES_H_
