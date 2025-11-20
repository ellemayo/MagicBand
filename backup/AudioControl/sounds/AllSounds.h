// Centralized header for all audio sounds
// Include this file to access all available sound effects

#ifndef ALL_SOUNDS_H
#define ALL_SOUNDS_H

// Include all individual sound headers
#include "box_opening.h"
#include "box_closing.h"
#include "chimes.h"
#include "sparkle.h"
#include "swoosh.h"
#include "wand.h"
#include "wand_sound.h"
#include "spell_sound.h"

// All sounds are now available as AudioData structs:
// - box_opening: Box opening sound effect
// - box_closing: Box closing sound effect
// - chimes: Chimes sound effect (48kHz, 192k samples)
// - sparkle: Sparkle/magical sound effect
// - swoosh: Swoosh sound effect
// - wand: Full wand activation sound (192000 samples)
// - wand_sound: Short wand sound (3200 samples)
// - spell_sound: Spell casting sound (4000 samples)

#endif // ALL_SOUNDS_H
