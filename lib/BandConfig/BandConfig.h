#ifndef BAND_CONFIG_H
#define BAND_CONFIG_H

#include <Arduino.h>
#include <FastLED.h>
#include <AudioControlDFPlayer.h>

// Band configuration structure with sound variation support
struct BandConfig {
  uint64_t band_id;              // RFID UID (unique identifier) - supports both 32-bit and 64-bit UIDs
  const char* name;              // Human-readable name for Home Assistant
  CRGB led_color;                // LED color to display
  uint8_t sound_files[3];        // Array of up to 3 sound file numbers
  uint8_t num_sounds;            // Number of sounds available for this band
  uint8_t current_sound_index;   // Current position in sound rotation
};

// External declarations - defined in main.cpp
extern BandConfig BAND_CONFIGS[];
extern const int NUM_BANDS;

// Helper function to find band configuration by ID (accepts both 32-bit and 64-bit)
BandConfig* find_band_config(uint64_t band_id);

#endif // BAND_CONFIG_H
