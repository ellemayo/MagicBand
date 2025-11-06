#ifndef AUDIO_CONTROL_DFPLAYER_H
#define AUDIO_CONTROL_DFPLAYER_H

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

// Pin definitions for DFPlayer Mini
#define DFPLAYER_RX_PIN 16  // Connect to DFPlayer TX
#define DFPLAYER_TX_PIN 17  // Connect to DFPlayer RX

// Audio file mapping
// NOTE: DFPlayer Mini requires files to be numbered (0001.mp3, 0002.mp3, etc.)
// Files are sorted alphabetically by the prepare_sd_card.ps1 script
// Currently using root directory (no folders)
//
// File order based on alphabetical sorting:
// 0001_box_closing.mp3      -> Track 1
// 0002_box_opening.mp3      -> Track 2
// 0003_chimes.mp3           -> Track 3
// 0004_fire-magic.mp3       -> Track 4
// 0005_healing-magic.mp3    -> Track 5
// 0006_healing-magic-2.mp3  -> Track 6
// 0007_magic-spell.mp3      -> Track 7
// 0008_magic-spell-2.mp3    -> Track 8
// 0009_magic-wand.mp3       -> Track 9
// 0010_sparkle.mp3          -> Track 10
// 0011_swoosh.mp3           -> Track 11
// 0012_wand.mp3             -> Track 12
// 0013_wind-magic.mp3       -> Track 13
enum SoundFile {
  SOUND_BOX_CLOSING = 1,
  SOUND_BOX_OPENING = 2,
  SOUND_CHIMES = 3,
  SOUND_FIRE_MAGIC = 4,
  SOUND_HEALING_MAGIC = 5,
  SOUND_HEALING_MAGIC_2 = 6,
  SOUND_MAGIC_SPELL = 7,
  SOUND_MAGIC_SPELL_2 = 8,
  SOUND_MAGIC_WAND = 9,
  SOUND_SPARKLE = 10,
  SOUND_SWOOSH = 11,
  SOUND_WAND = 12,
  SOUND_WIND_MAGIC = 13
};

// Volume settings (0-30)
#define DEFAULT_VOLUME 25
#define MAX_VOLUME 30
#define MIN_VOLUME 0

// Setup and control functions
bool setup_audio_dfplayer();
void set_volume(uint8_t volume);
uint8_t get_volume();
bool is_audio_playing();
void stop_audio();

// Sound playback functions - simplified!
bool play_sound_file(uint8_t file_number);
bool play_sound_from_folder(uint8_t folder_number, uint8_t file_number);

// DFPlayer status functions
bool dfplayer_is_ready();
uint16_t get_file_count();
void print_dfplayer_detail(uint8_t type, int value);

#endif
