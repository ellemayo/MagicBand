# MagicBand ESP32 Project - AI Coding Agent Instructions

## Project Overview
This is an ESP32-based magical interactive system that responds to **RFID bands or IR wands** with synchronized LED lighting, servo-controlled mechanical actions, and tone-based audio playback. The system creates immersive experiences by detecting specific wand/band IDs and triggering unique audiovisual responses for each character.

## Architecture & Component Structure

### Modular Library Design
Components are organized as self-contained libraries in `lib/` with clear separation of concerns:
- **IRControl**: IR wand protocol decoding with custom bit manipulation (legacy/optional)
- **RFIDControl**: MFRC522-based RFID band detection (alternative to IR)
- **LEDControl**: FastLED-based RGB strip control with color sequences  
- **ServoControl**: Physical mechanism control (lid/door movement)
- **AudioControl**: ESP32 DAC-based tone generation with mathematical synthesis

### Main Control Flow Pattern (`src/main.cpp`)
```cpp
// Core loop: IR/RFID detection → cooldown check → character dispatch → multi-component activation
uint32_t wand_id = loop_ir();     // IR version
// OR
uint32_t band_id = loop_rfid();   // RFID version

if (id != 0 && current_time - last_activation >= COOLDOWN_PERIOD) {
    set_color(character_color);          // LEDs
    delay(50);                           // Power spike prevention
    play_character_sound(id);            // Audio (tone-based)
    toggle_lid();                        // Servo
}
```

## Hardware Integration Patterns

### Pin Assignments (defined in component headers)
**IR Version (Default):**
- IR Receiver: GPIO14 (`IR_RECEIVE_PIN`)
- LED Strip: GPIO3 (`DATA_PIN`) 
- Servo Motor: GPIO18 (`SERVO_PIN`)
- Audio Output: GPIO25 (ESP32 DAC Channel 1)

**RFID Version (Alternative):**
- RFID SDA/SS: GPIO5 (`RFID_SS_PIN`)
- RFID RST: GPIO22 (`RFID_RST_PIN`)
- RFID SCK: GPIO18 (hardware SPI - required)
- RFID MOSI: GPIO23 (hardware SPI - required)
- RFID MISO: GPIO19 (hardware SPI - required)
- LED Strip: GPIO13 (`DATA_PIN`)
- DFPlayer TX: GPIO16 (ESP32 Serial2 RX)
- DFPlayer RX: GPIO17 (ESP32 Serial2 TX)
- No servo in this configuration

### Power Management Strategy
- **Critical**: 50ms delay between LED and servo activation prevents brown-out
- Brownout detector workaround available in `platformio.ini` (commented)
- Audio DAC initialized to silence (128) to prevent startup buzzing

## Development Workflows

### Building & Uploading
```bash
# PlatformIO CLI (preferred)
pio run                    # Build only
pio run --target upload    # Build and upload

# VS Code: Use "PlatformIO Build" task from Command Palette
```

### Audio Content Pipeline
1. **Prepare**: WAV file (any format - will be analyzed)
2. **Convert**: `python tools/wav_to_tones.py input.wav output.h array_name`
3. **Integrate**: Add header to `lib/AudioControl/sounds/AllSoundsTones.h`
4. **Implement**: Call `play_arrayname_tones()` in character-specific functions

### Character System Extension
When adding new wands/characters:
1. Define wand ID constant in `IRControl.h` (e.g., `WAND_NEWCHAR`)
2. Add case in `main.cpp` switch statement with unique color/sound/action
3. Use existing tone function or create new one with wav_to_tones.py
4. Cooldown applies globally across all wands

## Project-Specific Conventions

### IR Protocol Handling (IRControl)
- Uses custom IR wand protocol decoder (`decodeMagiQuest()`)
- Wand IDs are extracted from 64-bit payload via union structure manipulation
- Protocol constants defined for timing analysis (MAGIQUEST_PERIOD, MARK/SPACE values)

### RFID Protocol Handling (RFIDControl)
- Uses MFRC522 library for Mifare RFID cards
- Band UIDs extracted from 4-byte (or 7/10-byte) card UIDs
- Union structure for flexible UID access (uint32_t or byte array)
- Supports Mifare Classic 1K/4K and Ultralight cards (most common for wristbands)
- Read range: 2-4cm (close proximity required)

### Memory Management for Audio
- Audio generated as pure sine wave tones in real-time
- Tone sequences defined as simple frequency/duration arrays
- Total audio code footprint: ~3KB (vs 15KB+ for WAV files)

### Error Handling Philosophy
- Serial debug output for major state changes and wand activations
- Graceful degradation: system continues if individual components fail
- Hardware initialization order matters: audio DAC must be silenced early

## Integration Points

### Component Communication
- All components expose `setup_*()` and operational functions
- Main loop orchestrates timing and sequences across components  
- No direct inter-component dependencies (all go through main.cpp)

### External Dependencies
- `FastLED@^3.10.3`: RGB LED control with CRGB color objects
- `IRremote@^4.5.0`: Base IR protocol handling (extended for custom protocol) - IR version only
- `MFRC522@^1.4.11`: RFID RC522 reader support - RFID version only
- `ESP32Servo@^3.0.9`: Hardware servo control with position tracking

### Debug & Monitoring
- Serial output at 115200 baud for initialization and wand detection
- Wand activations and cooldowns logged
- Use Serial Monitor to verify IR detection and troubleshoot hardware

## Key Files for Understanding System
- `src/main.cpp`: Central orchestration and character behavior mapping (IR version)
- `src/main_rfid_example.cpp`: RFID version example implementation
- `lib/IRControl/IRControl.cpp`: IR protocol implementation details
- `lib/RFIDControl/RFIDControl.cpp`: MFRC522 RFID implementation
- `lib/AudioControl/sounds/AllSoundsTones.h`: All tone-based sound effects
- `docs/RFID_MIGRATION_GUIDE.md`: Complete guide for IR→RFID migration
- `README.md`: Current system documentation and usage guide
- `platformio.ini`: Hardware configuration and power management settings