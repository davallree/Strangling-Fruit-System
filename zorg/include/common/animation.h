#ifndef NEW_ZORG_COMMON_ANIMATION_H_
#define NEW_ZORG_COMMON_ANIMATION_H_
#ifndef INCLUDE_ANIMATION_H_
#define INCLUDE_ANIMATION_H_

#include <FastLED.h>

class Animation {
 public:
  virtual void animate(CRGB& led) = 0;
};

class Animator {
 public:
  Animator();

 private:
  CRGB led = CRGB::Black;
};

#endif // INCLUDE_ANIMATION_H_


#endif // NEW_ZORG_COMMON_ANIMATION_H_
