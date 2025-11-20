// Generated from assets/audio/swoosh.wav
// 10 tones, 2.00s total
// Original duration: 2.00s

#ifndef SWOOSH_TONES_H
#define SWOOSH_TONES_H

#include <AudioControl.h>

// Generated tone sequence: 10 tones, 2.00s total
void play_swoosh_tones() {
  // Sequence of 10 tones
  play_tone(1047, 100);  // 1047Hz for 100ms
  play_tone(1560, 50);  // 1560Hz for 50ms
  play_tone(1047, 150);  // 1047Hz for 150ms
  delay(10);  // Brief pause
  play_tone(2000, 50);  // 2000Hz for 50ms
  play_tone(1560, 150);  // 1560Hz for 150ms
  delay(10);  // Brief pause
  play_tone(1047, 50);  // 1047Hz for 50ms
  play_tone(2100, 50);  // 2100Hz for 50ms
  play_tone(1560, 150);  // 1560Hz for 150ms
  delay(10);  // Brief pause
  play_tone(6300, 50);  // 6300Hz for 50ms
  play_tone(1560, 50);  // 1560Hz for 50ms
}

#endif
