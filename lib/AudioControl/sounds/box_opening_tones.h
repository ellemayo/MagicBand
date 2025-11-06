// Generated from assets/audio/box_opening.wav
// 4 tones, 1.20s total
// Original duration: 1.20s

#ifndef BOX_OPENING_TONES_H
#define BOX_OPENING_TONES_H

#include <AudioControl.h>

// Generated tone sequence: 4 tones, 1.20s total
void play_box_opening_tones() {
  // Sequence of 4 tones
  play_tone(200, 100);  // 200Hz for 100ms
  play_tone(740, 100);  // 740Hz for 100ms
  play_tone(100, 50);  // 100Hz for 50ms
  play_tone(60, 50);  // 60Hz for 50ms
}

#endif
