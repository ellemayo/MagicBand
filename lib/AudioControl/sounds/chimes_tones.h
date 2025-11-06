// Generated from assets/audio/chimes.wav
// 14 tones, 4.00s total
// Original duration: 4.00s

#ifndef CHIMES_TONES_H
#define CHIMES_TONES_H

#include <AudioControl.h>

// Generated tone sequence: 14 tones, 4.00s total
void play_chimes_tones() {
  // Sequence of 14 tones
  play_tone(1397, 50);  // 1397Hz for 50ms
  play_tone(1780, 50);  // 1780Hz for 50ms
  play_tone(2000, 50);  // 2000Hz for 50ms
  play_tone(1780, 50);  // 1780Hz for 50ms
  play_tone(2360, 50);  // 2360Hz for 50ms
  play_tone(2660, 200);  // 2660Hz for 200ms
  delay(10);  // Brief pause
  play_tone(2820, 50);  // 2820Hz for 50ms
  play_tone(2360, 50);  // 2360Hz for 50ms
  play_tone(2820, 100);  // 2820Hz for 100ms
  play_tone(2660, 250);  // 2660Hz for 250ms
  delay(10);  // Brief pause
  play_tone(2360, 100);  // 2360Hz for 100ms
  play_tone(1780, 100);  // 1780Hz for 100ms
  play_tone(2360, 100);  // 2360Hz for 100ms
  play_tone(1397, 100);  // 1397Hz for 100ms
}

#endif
