# MagiQuest ESP32 Project - AI Coding Agent Instructions

## Project Overview
This is an ESP32-based magical interactive system that responds to MagiQuest IR wands with synchronized LED lighting, servo-controlled mechanical actions, and tone-based audio playback. The system creates immersive experiences by detecting specific wand IDs and triggering unique audiovisual responses for each character.

## Architecture & Component Structure

### Modular Library Design
Components are organized as self-contained libraries in `lib/` with clear separation of concerns:
- **IRControl**: MagiQuest protocol IR decoding with custom bit manipulation
- **LEDControl**: FastLED-based RGB strip control with color sequences  
- **ServoControl**: Physical mechanism control (lid/door movement)
- **AudioControl**: ESP32 DAC-based tone generation with mathematical synthesis

### Main Control Flow Pattern (`src/main.cpp`)
```cpp
// Core loop: IR detection → cooldown check → character dispatch → multi-component activation
uint32_t wand_id = loop_ir();
if (wand_id != 0 && current_time - last_activation >= COOLDOWN_PERIOD) {
    set_color(character_color);          // LEDs
    delay(50);                           // Power spike prevention
    play_character_sound(wand_id);       // Audio (tone-based)
    toggle_lid();                        // Servo
}
```

## Hardware Integration Patterns

### Pin Assignments (defined in component headers)
- IR Receiver: GPIO14 (`IR_RECEIVE_PIN`)
- LED Strip: GPIO3 (`DATA_PIN`) 
- Servo Motor: GPIO18 (`SERVO_PIN`)
- Audio Output: GPIO25 (ESP32 DAC Channel 1)

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

### IR Protocol Handling
- Uses custom MagiQuest protocol decoder (`decodeMagiQuest()`)
- Wand IDs are extracted from 64-bit payload via union structure manipulation
- Protocol constants defined for timing analysis (MAGIQUEST_PERIOD, MARK/SPACE values)

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
- `IRremote@^4.5.0`: Base IR protocol handling (extended for MagiQuest)
- `ESP32Servo@^3.0.9`: Hardware servo control with position tracking

### Debug & Monitoring
- Serial output at 115200 baud for initialization and wand detection
- Wand activations and cooldowns logged
- Use Serial Monitor to verify IR detection and troubleshoot hardware

## Key Files for Understanding System
- `src/main.cpp`: Central orchestration and character behavior mapping
- `lib/IRControl/IRControl.cpp`: MagiQuest protocol implementation details
- `lib/AudioControl/sounds/AllSoundsTones.h`: All tone-based sound effects
- `README.md`: Current system documentation and usage guide
- `platformio.ini`: Hardware configuration and power management settings