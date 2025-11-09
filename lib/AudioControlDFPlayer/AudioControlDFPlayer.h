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
// File order based on alphabetical sorting of assets/mp3_for_sd/:
// 0001_chime.mp3           -> Track 1
// 0002_error.mp3           -> Track 2  
// 0003_excellent.mp3       -> Track 3
// 0004_foolish.mp3         -> Track 4
// 0005_hello.mp3           -> Track 5
// 0006_operational.mp3     -> Track 6
// 0007_startours.mp3       -> Track 7
enum SoundFile {
  SOUND_CHIME = 1,
  SOUND_ERROR = 2,
  SOUND_EXCELLENT = 3,
  SOUND_FOOLISH = 4,
  SOUND_HELLO = 5,
  SOUND_OPERATIONAL = 6,
  SOUND_STARTOURS = 7,
  // RFID Detection feedback sounds
  SOUND_RFID_SUCCESS = SOUND_CHIME,      // Recognized band - chime sound
  SOUND_RFID_FAIL = SOUND_ERROR          // Unknown band - error sound
};

// Volume settings (0-30)
#define DEFAULT_VOLUME 24
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
