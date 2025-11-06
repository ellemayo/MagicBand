# DFPlayer Mini Audio System

## Overview
This implementation replaces the DAC-based tone generation with the **DFPlayer Mini MP3 player module**, which plays high-quality audio files from an SD card. This provides much better sound quality compared to synthesized tones.

## Hardware Setup

### DFPlayer Mini Connections
Connect the DFPlayer Mini to your ESP32 as follows:

| DFPlayer Pin | ESP32 Pin | Description |
|--------------|-----------|-------------|
| VCC          | 5V or 3.3V| Power supply (can use either voltage) |
| GND          | GND       | Ground |
| TX           | GPIO16    | Serial transmit (DFPlayer → ESP32) |
| RX           | GPIO17    | Serial receive (ESP32 → DFPlayer) |
| SPK1         | Speaker + | Speaker positive terminal |
| SPK2         | Speaker - | Speaker negative terminal |

**Important Notes:**
- The DFPlayer Mini can work with either 3.3V or 5V power
- Use a 1kΩ resistor between ESP32 TX (GPIO17) and DFPlayer RX for safety (optional but recommended)
- The SPK pins are amplified outputs - connect directly to a speaker (3-5W, 4-8Ω)
- Alternatively, use the DAC_L and DAC_R pins with an external amplifier

### SD Card Preparation

1. **Format the SD card:**
   - Must be FAT32 format
   - Recommended: 32GB or smaller (FAT32 limitation)
   - Use a quality SD card for reliable playback

2. **Audio file requirements:**
   - Format: MP3 or WAV
   - Sample rate: 44.1kHz or 48kHz recommended
   - Bit rate: 128kbps or higher for MP3
   - Naming: Files must be named numerically: `0001.mp3`, `0002.mp3`, etc.

3. **File mapping (as defined in code):**
   ```
   0001.mp3 = Startup sound
   0002.mp3 = Sparkle sound
   0003.mp3 = Chimes sound
   0004.mp3 = Wand sound
   0005.mp3 = Swoosh sound
   0006.mp3 = Box opening sound
   0007.mp3 = Box closing sound
   0008.mp3 = Spell sound
   ```

4. **Copy files to SD card:**
   - Place audio files in the root directory (not in folders)
   - File names must be exactly as shown above (0001.mp3, not 1.mp3 or 001.mp3)
   - You can use the audio files from `assets/audio/` folder in this project

## Software Integration

### Option 1: Switch Completely to DFPlayer
To use DFPlayer exclusively, modify `src/main.cpp`:

```cpp
// Replace this:
#include <AudioControl.h>

// With this:
#include <AudioControlDFPlayer.h>

// In setup():
setup_audio_dfplayer();  // Instead of setup_audio()

// Replace sound function calls:
play_startup_sound_dfplayer();  // Instead of play_startup_sound()
play_sparkle_dfplayer();        // Instead of play_sparkle_tones()
play_chimes_dfplayer();         // Instead of play_chimes_tones()
play_wand_dfplayer();           // Instead of play_wand_tones()
play_swoosh_dfplayer();         // Instead of play_swoosh_tones()
play_box_opening_dfplayer();    // Instead of play_box_opening_tones()
play_box_closing_dfplayer();    // Instead of play_box_closing_tones()
```

### Option 2: Keep Both Systems (Testing)
You can keep both audio systems and switch between them:

```cpp
#include <AudioControl.h>
#include <AudioControlDFPlayer.h>

// Define which system to use
#define USE_DFPLAYER true

void setup() {
  // ... other setup code ...
  
  if (USE_DFPLAYER) {
    setup_audio_dfplayer();
  } else {
    setup_audio();  // DAC tones
  }
}

// In your sound playback code:
if (USE_DFPLAYER) {
  play_sparkle_dfplayer();
} else {
  play_sparkle_tones();
}
```

## API Reference

### Setup Functions
- `bool setup_audio_dfplayer()` - Initialize DFPlayer, returns true on success
- `bool dfplayer_is_ready()` - Check if DFPlayer is initialized

### Playback Functions
- `bool play_sound_file(uint8_t file_number)` - Play specific file by number
- `bool play_startup_sound_dfplayer()` - Play startup sound (file 0001.mp3)
- `bool play_sparkle_dfplayer()` - Play sparkle sound (file 0002.mp3)
- `bool play_chimes_dfplayer()` - Play chimes sound (file 0003.mp3)
- `bool play_wand_dfplayer()` - Play wand sound (file 0004.mp3)
- `bool play_swoosh_dfplayer()` - Play swoosh sound (file 0005.mp3)
- `bool play_box_opening_dfplayer()` - Play box opening sound (file 0006.mp3)
- `bool play_box_closing_dfplayer()` - Play box closing sound (file 0007.mp3)
- `bool play_spell_dfplayer()` - Play spell sound (file 0008.mp3)

### Control Functions
- `void set_volume(uint8_t volume)` - Set volume (0-30)
- `uint8_t get_volume()` - Get current volume
- `bool is_audio_playing()` - Check if audio is currently playing
- `void stop_audio()` - Stop current playback
- `uint16_t get_file_count()` - Get number of files on SD card

## Troubleshooting

### DFPlayer Not Initializing
1. Check serial connections (TX/RX must be crossed: ESP32 TX → DFPlayer RX)
2. Verify power supply is adequate (DFPlayer draws ~20-50mA idle, up to 200mA during playback)
3. Ensure SD card is properly inserted and formatted as FAT32
4. Try removing and reinserting the SD card
5. Check Serial Monitor for detailed error messages

### No Sound Output
1. Verify speaker is connected to SPK1 and SPK2 pins
2. Check volume setting (default is 25 out of 30)
3. Ensure audio files are in correct format (MP3 or WAV)
4. Verify file naming is correct (0001.mp3, not 1.mp3)
5. Try `set_volume(30)` to test at maximum volume

### Audio Cuts Out or Sounds Distorted
1. Check power supply - DFPlayer needs stable power during playback
2. Use a capacitor (100-470µF) across VCC and GND pins
3. Verify SD card is not corrupted (try reformatting)
4. Use higher quality audio files (higher bitrate)
5. Ensure speaker impedance matches (4-8Ω recommended)

### Files Not Found
1. Confirm files are in root directory (not in folders)
2. Check file naming exactly matches: `0001.mp3`, `0002.mp3`, etc.
3. SD card must be FAT32 format
4. Try using a different SD card
5. Check file count with `get_file_count()` function

## Volume Control
The DFPlayer Mini supports 30 volume levels (0-30):
- `0` = Mute
- `15` = Half volume
- `25` = Default (comfortable level)
- `30` = Maximum volume

Adjust volume in code:
```cpp
set_volume(20);  // Set to level 20
uint8_t current = get_volume();  // Read current level
```

## Advantages Over DAC Tones
1. **Much better sound quality** - plays actual audio files instead of synthetic tones
2. **Professional sounds** - use any MP3/WAV files you want
3. **Easier content creation** - no need to convert audio to tone sequences
4. **Built-in amplification** - can drive speakers directly
5. **SD card storage** - easily update sounds without reflashing firmware

## Pin Conflicts
The DFPlayer uses GPIO16 and GPIO17 for serial communication. If these pins are used elsewhere in your project, you can change them in `AudioControlDFPlayer.h`:

```cpp
#define DFPLAYER_RX_PIN 16  // Change these if needed
#define DFPLAYER_TX_PIN 17
```

ESP32 has 3 hardware serial ports, and this implementation uses UART2. Make sure these pins aren't used for other purposes.

## Converting Audio Files
To prepare your audio files:

1. **Use online converter or tool like Audacity:**
   - Export as MP3 (128kbps or higher) or WAV
   - Mono or stereo (DFPlayer handles both)
   - 44.1kHz sample rate recommended

2. **Rename files:**
   ```powershell
   # Example PowerShell commands
   Rename-Item "sparkle.mp3" "0002.mp3"
   Rename-Item "chimes.mp3" "0003.mp3"
   ```

3. **Copy to SD card root directory**

## Example Usage
```cpp
#include <AudioControlDFPlayer.h>

void setup() {
  Serial.begin(115200);
  
  // Initialize DFPlayer
  if (setup_audio_dfplayer()) {
    Serial.println("DFPlayer ready!");
    
    // Set volume
    set_volume(25);
    
    // Play startup sound
    play_startup_sound_dfplayer();
  } else {
    Serial.println("DFPlayer initialization failed!");
  }
}

void loop() {
  // Play different sounds based on events
  if (wand_detected) {
    play_wand_dfplayer();
    delay(2000);  // Wait for sound to finish
  }
}
```

## Future Enhancements
Possible improvements to this implementation:
1. Add non-blocking playback with callbacks
2. Support for multiple playlists or folders
3. Dynamic volume adjustment based on ambient noise
4. Shuffle/random playback modes
5. Fade in/out effects
6. Integration with Home Assistant for remote volume control
