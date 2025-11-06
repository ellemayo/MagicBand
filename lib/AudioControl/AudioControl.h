#ifndef AUDIO_CONTROL_H
#define AUDIO_CONTROL_H

#include <Arduino.h>
#include <driver/dac.h>

struct AudioData {
  const unsigned char* data;
  unsigned int length;
  unsigned int sample_rate;
};

void setup_audio();
bool play_sound();

void play_startup_sound();
void play_wand_sound();
void play_spell_sound();
void play_tone(int frequency, int duration);
void play_audio(const AudioData& audio);
void play_audio_data(const unsigned char* audio_data, unsigned int length, unsigned int sample_rate);

// Tone-based sound effects
void play_sparkle_tones();
void play_chimes_tones();
void play_wand_tones();
void play_swoosh_tones();
void play_box_opening_tones();
void play_box_closing_tones();

#endif