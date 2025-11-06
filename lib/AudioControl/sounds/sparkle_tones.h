// Generated from assets/audio/sparkle.wav
// 9 tones, 2.00s total
// Original duration: 2.00s

#ifndef SPARKLE_TONES_H
#define SPARKLE_TONES_H

#include <AudioControl.h>

// Generated tone sequence: 9 tones, 2.00s total
void play_sparkle_tones() {
  // Sequence of 9 tones
  play_tone(349, 50);  // 349Hz for 50ms
  play_tone(440, 50);  // 440Hz for 50ms
  play_tone(523, 50);  // 523Hz for 50ms
  play_tone(659, 50);  // 659Hz for 50ms
  play_tone(698, 150);  // 698Hz for 150ms
  delay(10);  // Brief pause
  play_tone(1397, 100);  // 1397Hz for 100ms
  play_tone(1580, 50);  // 1580Hz for 50ms
  play_tone(2100, 50);  // 2100Hz for 50ms
  play_tone(1397, 150);  // 1397Hz for 150ms
  delay(10);  // Brief pause
}

#endif
