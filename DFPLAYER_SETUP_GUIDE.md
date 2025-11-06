# DFPlayer Mini MP3 Setup Guide

## Overview
This guide explains how to prepare your SD card with MP3 files for use with the DFPlayer Mini audio module.

## File Structure on SD Card
Files are placed in the **root directory** of the SD card with 4-digit numeric prefixes:
```
E:\
├── 0001_box_closing.mp3
├── 0002_box_opening.mp3
├── 0003_chimes.mp3
├── 0004_sparkle.mp3
├── 0005_swoosh.mp3
└── 0006_wand.mp3
```

## Sound File Mappings
The code maps these files to constants (defined in `lib/AudioControlDFPlayer/AudioControlDFPlayer.h`):

| Track # | Filename | Code Constant | Usage |
|---------|----------|---------------|-------|
| 1 | 0001_box_closing.mp3 | `SOUND_BOX_CLOSING` | Box closing sound |
| 2 | 0002_box_opening.mp3 | `SOUND_BOX_OPENING` | Box opening sound |
| 3 | 0003_chimes.mp3 | `SOUND_CHIMES` | Chimes sound, also used for startup |
| 4 | 0004_sparkle.mp3 | `SOUND_SPARKLE` | Sparkle sound |
| 5 | 0005_swoosh.mp3 | `SOUND_SWOOSH` | Swoosh sound |
| 6 | 0006_wand.mp3 | `SOUND_WAND` | Wand sound |

## Wand Configuration
Each wand rotates through 3 different sounds on activation:

### Wand 1 (Blue - August)
- Chimes → Swoosh → Box Opening (rotates)

### Wand 2 (Green - Ophelia)
- Sparkle → Wand → Chimes (rotates)

### Wand 3 (Purple - Evalette)
- Wand → Swoosh → Sparkle (rotates)

## Setup Steps

### Step 1: Convert WAV to MP3
Run the conversion script to convert your WAV files to DFPlayer-compatible MP3 format:
```powershell
.\tools\convert_audio_for_dfplayer.ps1
```

This script:
- Reads WAV files from `assets\audio\`
- Converts to MP3 at 44.1kHz, mono, 128kbps CBR
- Saves to `assets\mp3_for_sd\`
- Excludes `-min.wav` files automatically

**Output:** MP3 files in `assets\mp3_for_sd\`

### Step 2: Prepare SD Card
Format your SD card as **FAT32** (required by DFPlayer Mini).

Run the SD card preparation script:
```powershell
.\tools\prepare_sd_card.ps1 -DriveLetter F
```
Replace `F` with your SD card's drive letter.

This script:
- Clears any existing numbered MP3 files from the root
- Copies MP3 files from `assets\mp3_for_sd\`
- Renames them with 4-digit prefixes (0001_, 0002_, etc.)
- Sorts files alphabetically to ensure consistent numbering

**Output:** Numbered MP3 files in the root of your SD card

### Step 3: Insert SD Card
1. Safely eject the SD card from your computer
2. Insert it into the DFPlayer Mini module
3. Power on the ESP32

### Step 4: Verify Operation
Check the Serial Monitor (115200 baud) for initialization messages:
```
DFPlayer Mini initialized successfully!
Volume set to: 25
Files on SD card: 6
```

## Audio File Requirements

### Format Specifications
- **Sample Rate:** 44.1kHz (DFPlayer Mini's native rate)
- **Bitrate:** 128kbps CBR (Constant Bit Rate recommended)
- **Channels:** Mono (reduces stuttering and file size)
- **Format:** MP3

### SD Card Requirements
- **Format:** FAT32 (not exFAT or NTFS)
- **Capacity:** Up to 32GB recommended
- **Speed:** Class 4 or higher

## Troubleshooting

### Audio Stutters or Skips
1. **Check sample rate:** Must be 44.1kHz, not 48kHz
2. **Use CBR encoding:** Variable bitrate (VBR) can cause issues
3. **Convert to mono:** Stereo files are larger and more prone to stuttering
4. **Try a different SD card:** Some cards have compatibility issues
5. **Check power supply:** Ensure DFPlayer has stable 5V power

### Files Not Playing
1. **Verify FAT32 format:** DFPlayer doesn't support exFAT or NTFS
2. **Check file numbering:** Must have 4-digit prefix (0001_, 0002_, etc.)
3. **Root directory only:** Don't use folders unless configured for folder playback
4. **Serial Monitor:** Check for error messages

### DFPlayer Not Initializing
1. **Check wiring:**
   - ESP32 GPIO16 → DFPlayer TX (Pin 3)
   - ESP32 GPIO17 → DFPlayer RX (Pin 2) with 1kΩ resistor recommended
   - VCC: 3.3-5V
   - GND: Common ground
2. **SD card inserted:** DFPlayer needs SD card to initialize
3. **Power supply:** Ensure adequate current (100-200mA when playing)

## Adding New Sounds

### To add a new sound:
1. Add the WAV file to `assets\audio\`
2. Run `.\tools\convert_audio_for_dfplayer.ps1`
3. Update `AudioControlDFPlayer.h` to add new enum value
4. Check alphabetical order (track numbers are auto-assigned by filename)
5. Run `.\tools\prepare_sd_card.ps1 -DriveLetter X`
6. Update `main.cpp` wand configurations if needed

### Important Note on Track Numbering
Track numbers are assigned **alphabetically by filename**. If you add a file like `aardvark.mp3`, it will become track 1, shifting all other files down by one number. Plan your filenames carefully!

## Hardware Recommendations

### For Best Audio Quality
1. **Power isolation:** Use separate 5V supply for DFPlayer
2. **Level shifting:** Add 1kΩ resistor on ESP32 TX → DFPlayer RX
3. **Decoupling capacitor:** 100µF on DFPlayer VCC/GND
4. **Quality SD card:** Use name-brand Class 10 cards
5. **Proper speaker:** 8Ω 0.5-3W speaker recommended

## Code References
- Sound mappings: `lib/AudioControlDFPlayer/AudioControlDFPlayer.h`
- Wand configurations: `src/main.cpp` (WAND_CONFIGS array)
- Playback logic: `lib/AudioControlDFPlayer/AudioControlDFPlayer.cpp`
