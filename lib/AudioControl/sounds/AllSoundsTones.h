// All tone-based sound effects in one header
// This provides a simple alternative to WAV playback that works
// perfectly with the ESP32 DAC + LM386 amplifier without hardware modifications

#ifndef ALL_SOUNDS_TONES_H
#define ALL_SOUNDS_TONES_H

#include "sparkle_tones.h"
#include "chimes_tones.h"
#include "wand_tones.h"
#include "swoosh_tones.h"
#include "box_opening_tones.h"
#include "box_closing_tones.h"

// All tone-based sound functions are now available:
// - play_sparkle_tones()     // 9 tones, 2.00s - ascending sparkle effect
// - play_chimes_tones()      // 14 tones, 4.00s - bell-like chimes
// - play_wand_tones()        // 32 tones, 4.00s - magical wand whoosh
// - play_swoosh_tones()      // 10 tones, 2.00s - quick swoosh sound
// - play_box_opening_tones() // 4 tones, 1.20s - low creaking open
// - play_box_closing_tones() // 5 tones, 1.20s - descending close

#endif
