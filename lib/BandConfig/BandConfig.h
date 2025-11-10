#ifndef BAND_CONFIG_H
#define BAND_CONFIG_H

#include <Arduino.h>
#include <FastLED.h>
#include <AudioControlDFPlayer.h>

// Band configuration structure with sound variation support
struct BandConfig {
  uint32_t band_id;              // RFID UID (unique identifier)
  const char* name;              // Human-readable name for Home Assistant
  CRGB led_color;                // LED color to display
  uint8_t sound_files[3];        // Array of up to 3 sound file numbers
  uint8_t num_sounds;            // Number of sounds available for this band
  uint8_t current_sound_index;   // Current position in sound rotation
};

// External declarations - defined in main.cpp
extern BandConfig BAND_CONFIGS[];
extern const int NUM_BANDS;

// Helper function to find band configuration by ID
BandConfig* find_band_config(uint32_t band_id);

#endif // BAND_CONFIG_H
