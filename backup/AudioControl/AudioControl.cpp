
#include <AudioControl.h>
#include <DebugConfig.h>
// #include "sounds/AllSounds.h"  // Commented out to save flash space (uses ~15KB)
#include "sounds/AllSoundsTones.h"  // Using tone-based sounds instead (uses ~3KB)

// Simple tone generation using ESP32 DAC
void setup_audio() {
  // Enable DAC channel 1 (GPIO25)
  dac_output_enable(DAC_CHANNEL_1);
  // Set to neutral/silence level immediately to prevent buzzing
  dac_output_voltage(DAC_CHANNEL_1, 128);
  DEBUG_PRINTLN("Audio DAC initialized on GPIO25 - silence set");
}

void play_tone(int frequency, int duration) {
  const int sampleRate = 8000;
  const int samples = sampleRate * duration / 1000;
  
  for (int i = 0; i < samples; i++) {
    // Generate sine wave
    float angle = 2.0 * PI * frequency * i / sampleRate;
    int value = (int)(127 * sin(angle)) + 128; // Scale to 0-255 for DAC
    
    // Output to DAC
    dac_output_voltage(DAC_CHANNEL_1, value);
    
    // Small delay to maintain sample rate
    delayMicroseconds(1000000 / sampleRate);
  }
  
  // Silence
  dac_output_voltage(DAC_CHANNEL_1, 128);
}

void play_audio_data(const unsigned char* audio_data, unsigned int length, unsigned int sample_rate) {
  DEBUG_PRINT("Playing audio: ");
  DEBUG_PRINT(length);
  DEBUG_PRINT(" samples @ ");
  DEBUG_PRINT(sample_rate);
  DEBUG_PRINTLN(" Hz");
  
  // Calculate precise timing
  unsigned long start_time = micros();
  unsigned long sample_period_us = 1000000UL / sample_rate;
  
  // Play audio with accurate timing
  for (unsigned int i = 0; i < length; i++) {
    // Calculate target time for this sample
    unsigned long target_time = start_time + (i * sample_period_us);
    
    // Read sample from flash memory (PROGMEM)
    unsigned char sample = pgm_read_byte(&audio_data[i]);
    
    // Output to DAC
    dac_output_voltage(DAC_CHANNEL_1, sample);
    
    // Wait for precise sample timing
    while (micros() < target_time) {
      // Tight timing loop
    }
  }
  
  // Return to silence (center voltage)
  dac_output_voltage(DAC_CHANNEL_1, 128);
  
  float duration_ms = (micros() - start_time) / 1000.0;
  DEBUG_PRINT("Playback complete: ");
  DEBUG_PRINT(duration_ms, 1);
  DEBUG_PRINTLN(" ms");
}

// Simplified audio playback using AudioData struct
void play_audio(const AudioData& audio) {
  play_audio_data(audio.data, audio.length, audio.sample_rate);
}

// Legacy functions - simplified to use tones
bool play_sound() {
  DEBUG_PRINTLN("Playing magical sound");
  play_sparkle_tones();  // Using tone version to save space
  return true;
}

// Function to play different sounds for different wands
void play_wand_sound() {
  DEBUG_PRINTLN("Playing wand activation sound");
  play_wand_tones();  // Using tone version to save space
}

void play_spell_sound() {
  DEBUG_PRINTLN("Playing spell sound");
  play_sparkle_tones();  // Using tone version to save space
}

void play_startup_sound() {
  DEBUG_PRINTLN("Playing startup sound");
  play_swoosh_tones();
}

