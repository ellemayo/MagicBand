// Generated from assets/audio/box_closing.wav
// 5 tones, 1.20s total
// Original duration: 1.20s

#ifndef BOX_CLOSING_TONES_H
#define BOX_CLOSING_TONES_H

#include <AudioControl.h>

// Generated tone sequence: 5 tones, 1.20s total
void play_box_closing_tones() {
  // Sequence of 5 tones
  play_tone(392, 50);  // 392Hz for 50ms
  play_tone(554, 50);  // 554Hz for 50ms
  play_tone(2280, 50);  // 2280Hz for 50ms
  play_tone(466, 50);  // 466Hz for 50ms
  play_tone(277, 100);  // 277Hz for 100ms
}

#endif
