#include "AudioControlDFPlayer.h"
#include <DebugConfig.h>

// Create Serial port for DFPlayer communication
HardwareSerial DFPlayerSerial(2); // Use UART2

// DFPlayer Mini object
DFRobotDFPlayerMini myDFPlayer;

// Current volume setting
uint8_t current_volume = DEFAULT_VOLUME;

// Status flags
bool dfplayer_initialized = false;

/**
 * Initialize the DFPlayer Mini module
 * Sets up serial communication and configures the player
 */
bool setup_audio_dfplayer() {
  DEBUG_PRINTLN("Initializing DFPlayer Mini...");
  
  // Initialize serial communication with DFPlayer
  DFPlayerSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
  
  // Reduced delay - DFPlayer should respond in ~500ms if working
  // If it's not connected, no point waiting longer
  delay(500);
  
  // Connect to DFPlayer with timeout behavior
  if (!myDFPlayer.begin(DFPlayerSerial, true, true)) {
    DEBUG_PRINTLN("DFPlayer Mini initialization FAILED!");
    DEBUG_PRINTLN("Check connections:");
    DEBUG_PRINT("  RX Pin: "); DEBUG_PRINTLN(DFPLAYER_RX_PIN);
    DEBUG_PRINT("  TX Pin: "); DEBUG_PRINTLN(DFPLAYER_TX_PIN);
    DEBUG_PRINTLN("  VCC: 3.3-5V");
    DEBUG_PRINTLN("  GND: GND");
    DEBUG_PRINTLN("Verify SD card is inserted and formatted as FAT32");
    DEBUG_PRINTLN("System will continue without audio");
    return false;
  }
  
  DEBUG_PRINTLN("DFPlayer Mini initialized successfully!");
  
  // Reduced configuration delays - commands are queued anyway
  delay(100);
  
  // Set default volume
  myDFPlayer.volume(current_volume);
  delay(30);  // Reduced from 50ms
  DEBUG_PRINT("Volume set to: ");
  DEBUG_PRINTLN(current_volume);
  
  // Set equalizer (0=Normal, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass)
  myDFPlayer.EQ(0);  // Normal EQ
  delay(30);  // Reduced from 50ms
  
  // Set output device to SD card
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  
  // Reduced stabilization delay
  delay(200);
  
  // Get file count - don't block if this fails
  int file_count = myDFPlayer.readFileCounts();
  DEBUG_PRINT("Files on SD card: ");
  DEBUG_PRINTLN(file_count);
  
  if (file_count <= 0 || file_count == -1) {
    DEBUG_PRINTLN("WARNING: SD card read error or no files found!");
    DEBUG_PRINTLN("Possible causes:");
    DEBUG_PRINTLN("  - SD card not inserted");
    DEBUG_PRINTLN("  - SD card not formatted as FAT32");
    DEBUG_PRINTLN("  - No audio files on SD card");
    DEBUG_PRINTLN("  - DFPlayer communication error");
    DEBUG_PRINTLN("Audio playback may not work!");
    // Don't return false - let system continue
  }
  
  dfplayer_initialized = true;
  DEBUG_PRINTLN("DFPlayer setup complete");
  return true;
}

/**
 * Set playback volume
 * @param volume Volume level (0-30)
 */
void set_volume(uint8_t volume) {
  if (!dfplayer_initialized) {
    DEBUG_PRINTLN("DFPlayer not initialized!");
    return;
  }
  
  if (volume > MAX_VOLUME) {
    volume = MAX_VOLUME;
  }
  
  current_volume = volume;
  myDFPlayer.volume(current_volume);
  
  DEBUG_PRINT("Volume set to: ");
  DEBUG_PRINTLN(current_volume);
}

/**
 * Get current volume setting
 * @return Current volume level (0-30)
 */
uint8_t get_volume() {
  return current_volume;
}

/**
 * Check if audio is currently playing
 * @return true if playing, false if idle
 */
bool is_audio_playing() {
  if (!dfplayer_initialized) return false;
  
  // DFPlayer Mini doesn't have a direct "is playing" status
  // We can check by reading the state
  int state = myDFPlayer.readState();
  // State: 1=playing, 2=paused, 512=stopped
  return (state == 1);
}

/**
 * Stop current playback
 */
void stop_audio() {
  if (!dfplayer_initialized) return;
  
  myDFPlayer.stop();
  DEBUG_PRINTLN("Audio playback stopped");
}

/**
 * Play a specific audio file from SD card
 * @param file_number File number (1-based, e.g., 0001.mp3, 0002.mp3)
 * @return true if successful, false otherwise
 */
bool play_sound_file(uint8_t file_number) {
  if (!dfplayer_initialized) {
    DEBUG_PRINTLN("DFPlayer not initialized!");
    return false;
  }
  
  DEBUG_PRINT("Playing file: ");
  DEBUG_PRINTLN(file_number);
  
  // Send play command
  myDFPlayer.play(file_number);
  
  // CRITICAL: Add delay to allow DFPlayer to buffer audio data
  // Without this delay, playback can stutter at the start
  // 100-150ms allows the DFPlayer to read from SD card and fill its buffer
  delay(100);
  
  // Note: DFPlayer often sends status messages (like type 11) after play commands
  // These are not errors - they're informational. We'll let the main error handler
  // catch any real issues. Checking immediately after play() often gives false positives.
  
  return true;
}

/**
 * Play a specific audio file from a folder on SD card
 * Allows better organization of audio files
 * @param folder_number Folder number (01-99)
 * @param file_number File number within folder (001-255)
 * @return true if successful, false otherwise
 */
bool play_sound_from_folder(uint8_t folder_number, uint8_t file_number) {
  if (!dfplayer_initialized) {
    DEBUG_PRINTLN("DFPlayer not initialized!");
    return false;
  }
  
  DEBUG_PRINT("Playing folder ");
  DEBUG_PRINT(folder_number);
  DEBUG_PRINT(" file ");
  DEBUG_PRINTLN(file_number);
  
  myDFPlayer.playFolder(folder_number, file_number);
  
  // Note: Same as play_sound_file - don't check for "errors" immediately after playback
  
  return true;
}

/**
 * Check if DFPlayer is ready
 * @return true if initialized, false otherwise
 */
bool dfplayer_is_ready() {
  return dfplayer_initialized;
}

/**
 * Get number of files on SD card
 * @return File count
 */
uint16_t get_file_count() {
  if (!dfplayer_initialized) return 0;
  return myDFPlayer.readFileCounts();
}

/**
 * Print DFPlayer error/status messages
 */
void print_dfplayer_detail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      DEBUG_PRINTLN("DFPlayer: Time Out!");
      break;
    case WrongStack:
      DEBUG_PRINTLN("DFPlayer: Wrong Stack!");
      break;
    case DFPlayerCardInserted:
      DEBUG_PRINTLN("DFPlayer: Card Inserted!");
      break;
    case DFPlayerCardRemoved:
      DEBUG_PRINTLN("DFPlayer: Card Removed!");
      break;
    case DFPlayerCardOnline:
      DEBUG_PRINTLN("DFPlayer: Card Online!");
      break;
    case DFPlayerPlayFinished:
      DEBUG_PRINT("DFPlayer: Finished playing file ");
      DEBUG_PRINTLN(value);
      break;
    case DFPlayerError:
      DEBUG_PRINT("DFPlayer Error: ");
      switch (value) {
        case Busy:
          DEBUG_PRINTLN("Card not found");
          break;
        case Sleeping:
          DEBUG_PRINTLN("Sleeping");
          break;
        case SerialWrongStack:
          DEBUG_PRINTLN("Serial wrong stack");
          break;
        case CheckSumNotMatch:
          DEBUG_PRINTLN("Checksum not match");
          break;
        case FileIndexOut:
          DEBUG_PRINTLN("File index out of bounds");
          break;
        case FileMismatch:
          DEBUG_PRINTLN("File mismatch");
          break;
        case Advertise:
          DEBUG_PRINTLN("Advertise");
          break;
        default:
          DEBUG_PRINT("Unknown error: ");
          DEBUG_PRINTLN(value);
          break;
      }
      break;
    default:
      // Many DFPlayer clones send undocumented status messages (like type 11, 12, 13, etc.)
      // These are typically benign status updates, not errors
      // Only log them if debugging is needed
      #ifdef DEBUG_DFPLAYER_MESSAGES
      DEBUG_PRINT("DFPlayer: Undocumented message type ");
      DEBUG_PRINT(type);
      DEBUG_PRINT(" value: ");
      DEBUG_PRINTLN(value);
      #endif
      break;
  }
}
