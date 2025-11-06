# MagiQuest ESP32 Interactive System

## System Overview

This ESP32-based magical interactive system responds to MagiQuest IR wands with synchronized LED lighting, servo-controlled mechanical actions, and tone-based audio playback. The system creates immersive experiences by detecting specific wand IDs and triggering unique audiovisual responses for each character.

### ✨ New Features
- **Sound Variation System**: Each wand now has 3 unique sounds that cycle on each activation
- **Home Assistant Integration**: Full IoT control via WiFi/MQTT (optional)

## Features

### Audio System
Uses **tone-based audio** generated mathematically via ESP32's DAC for clean, magical sounds:
- **Sparkle** - 9 tones, 2.0s - Rising magical effect
- **Chimes** - 14 tones, 4.0s - Bell-like sounds  
- **Wand** - 32 tones, 4.0s - Complex magical whoosh
- **Swoosh** - 10 tones, 2.0s - Quick movement sound
- **Box Opening** - 4 tones, 1.2s - Low creaking
- **Box Closing** - 5 tones, 1.2s - Descending close

### Character System with Sound Variations
Each wand has 3 unique sounds that rotate on each activation:

- **August** (Blue wand, ID: 20451)
  - Sound 1: Chimes
  - Sound 2: Swoosh
  - Sound 3: Box Opening

- **Ophelia** (Green wand, ID: 22171)
  - Sound 1: Sparkle
  - Sound 2: Wand
  - Sound 3: Chimes

- **Evalette** (Purple wand, ID: 22752)
  - Sound 1: Wand
  - Sound 2: Swoosh
  - Sound 3: Sparkle

### Home Assistant Integration (Optional)
Full IoT control and monitoring:
- **Controls**: Enable/disable system, adjust LED brightness, set cooldown time
- **Sensors**: Last wand used, activation count, uptime, lid state
- **Automations**: Schedule on/off times, brightness based on time of day, notifications

See **[HOME_ASSISTANT_SETUP.md](HOME_ASSISTANT_SETUP.md)** for complete setup guide.

### Visual Feedback
- Cooldown indicator: First LED pulses gently during 5-second cooldown period
- Character-specific colors displayed on full LED strip
- Smooth fade effects on lid close

## Technical Details
- **DAC Output:** GPIO25 (ESP32 DAC Channel 1)
- **Sample Rate:** 8000 Hz for tone generation
- **Waveform:** Pure sine waves
- **Memory Usage:** ~3KB for audio (vs 15KB+ for WAV files)
- **Debug Output:** Conditional compilation (can be disabled to save ~2KB flash)

## Converting Audio Files

To add new sounds, use the WAV-to-tone converter:

```bash
python tools/wav_to_tones.py <input.wav> <output.h> <name>
```

Example:
```bash
python tools/wav_to_tones.py assets/audio/mysound.wav lib/AudioControl/sounds/mysound_tones.h mysound
```

This will generate a header file with a `play_mysound_tones()` function.

## Hardware Setup

### Connections
- IR Receiver → GPIO14
- LED Strip → GPIO3
- Servo Motor → GPIO18
- Audio Output → GPIO25 → LM386 Amplifier → Speaker

### Power Notes
- 50ms delay between LED and servo activation prevents brown-out
- Audio DAC initialized to silence (128) to prevent startup noise

## Building and Uploading

```bash
# Build only
pio run

# Build and upload
pio run --target upload
```

Or use VS Code PlatformIO extension buttons.

## Adding New Wand Characters

1. Define wand ID in `lib/IRControl/IRControl.h`
2. Add case in `src/main.cpp` switch statement
3. Call appropriate tone function (or create new one)
4. Set LED color with `set_color(CRGB::Color)`

## Memory Constraints

ESP32 has 1.25MB flash limit. Current build uses tone-based sounds to stay well under this limit. If you need WAV file playback, see `docs/archive/` for historical documentation on hardware filtering requirements.

## File Structure

```
lib/AudioControl/
├── AudioControl.cpp          # Main audio implementation
├── AudioControl.h            # Audio interface
└── sounds/
    ├── AllSoundsTones.h      # Master header for all tones
    ├── sparkle_tones.h
    ├── chimes_tones.h
    ├── wand_tones.h
    ├── swoosh_tones.h
    ├── box_opening_tones.h
    └── box_closing_tones.h
```

## Additional Documentation

- **[HOME_ASSISTANT_SETUP.md](HOME_ASSISTANT_SETUP.md)** - Complete guide to WiFi/MQTT/Home Assistant integration
- **[IMPROVEMENTS_SUMMARY.md](IMPROVEMENTS_SUMMARY.md)** - Recent bug fixes and optimizations
- `TONE_SOUNDS_GUIDE.md` - Detailed guide to tone-based audio system
- `READY_TO_TEST.md` - Testing and evaluation guide
- `.github/copilot-instructions.md` - AI coding agent context
- `docs/archive/` - Historical documentation (WAV playback attempts, hardware fixes, etc.)

## Recent Updates

### Sound Variation System ✅
- Each wand now plays through 3 different sounds
- Sounds rotate automatically on each activation
- Prevents repetitiveness and adds variety

### Home Assistant Integration ✅  
- Full WiFi/MQTT connectivity
- Remote control of system enable/disable, brightness, cooldown time
- Real-time sensors for wand activity, activation count, uptime
- Automatic Home Assistant discovery
- Supports automations and notifications

### Bug Fixes & Optimizations ✅
- Added IR decode error handling
- Fixed power spike protection (50ms delay)
- Conditional debug output system (saves ~2KB when disabled)
- Extracted all magic numbers to named constants
- Added cooldown visual feedback (LED pulse)

See `IMPROVEMENTS_SUMMARY.md` for complete details.
