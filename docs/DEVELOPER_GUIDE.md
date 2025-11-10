# MagicBand Developer Guide

## Table of Contents
- [Architecture Overview](#architecture-overview)
- [Component Libraries](#component-libraries)
- [Data Flow](#data-flow)
- [Adding New Features](#adding-new-features)
- [Code Conventions](#code-conventions)
- [Build & Deploy](#build--deploy)
- [Debugging](#debugging)

## Architecture Overview

### Modular Library Design
The MagicBand system follows a modular architecture with self-contained libraries:

```
lib/
├── AudioControl/          # Tone-based audio via ESP32 DAC
├── AudioControlDFPlayer/  # DFPlayer Mini MP3 playback
├── BandConfig/            # Character/band configuration
├── DebugConfig/           # Debug output control
├── HomeAssistantControl/  # WiFi/MQTT/HA integration
├── IRControl/             # IR wand detection (original)
├── LEDControl/            # FastLED RGB strip control
├── OTAControl/            # Over-the-air updates
├── RFIDControl/           # RFID band detection (alternative)
├── ServoControl/          # Servo motor control
└── TelnetDebug/           # Remote debugging
```

### Component Interface Pattern
Each component follows a consistent interface:

```cpp
// Setup function - called once during boot
void setup_component();

// Loop function - called every iteration
[return_type] loop_component();

// Helper/utility functions as needed
void component_helper_function();
```

### Main Control Flow

```cpp
void setup() {
    Serial.begin(115200);
    
    // Initialize components in order
    setup_leds();
    setup_audio();       // or setup_dfplayer()
    setup_ir();          // or setup_rfid()
    setup_servo();       // optional
    setup_home_assistant();  // optional
    setup_ota();         // optional
}

void loop() {
    // Process non-blocking components
    loop_ota();
    loop_home_assistant();
    
    // Get input (IR or RFID)
    uint32_t id = loop_ir();  // or loop_rfid()
    
    // Check cooldown and process
    if (id != 0 && not_in_cooldown()) {
        trigger_effects(id);
    }
    
    // Visual feedback
    cooldown_pulse();
    
    delay(100);  // Main loop timing
}
```

## Component Libraries

### LED Control (`lib/LEDControl/`)

**Purpose**: Control WS2812B RGB LED strip with FastLED

**Key Functions**:
```cpp
void setup_leds();
void set_color(CRGB color);
void cooldown_pulse();
void accelerating_chase(CRGB color);
void fade_in_out(CRGB color, int fade_speed_ms = 20);
void flash_color(CRGB color, int num_flashes = 3, int flash_speed_ms = 200);
```

**Configuration** (`LEDControl.h`):
```cpp
#define DATA_PIN 13           // GPIO for LED data
#define NUM_LEDS 17           // Number of LEDs
#define LED_DEFAULT_BRIGHTNESS 10
#define LED_MAX_VOLTAGE 5
#define LED_MAX_MILLIAMPS 200
```

**Adding New Animations**:
1. Add function to `LEDControl.cpp`
2. Declare in `LEDControl.h`
3. Use non-blocking patterns (millis() timing)
4. Call `FastLED.show()` after changing LEDs

### Audio Control (`lib/AudioControl/`)

**Purpose**: Generate tone-based audio via ESP32 DAC

**Key Functions**:
```cpp
void setup_audio();
void play_chimes_tones();
void play_sparkle_tones();
void play_wand_tones();
// ... other tone functions
```

**How It Works**:
- Generates pure sine waves mathematically
- Uses ESP32 DAC (GPIO25) at 8kHz sample rate
- Tone sequences defined as frequency/duration arrays
- Very memory efficient (~3KB vs 15KB+ for WAV)

**Adding New Sounds**:
1. Use `tools/wav_to_tones.py` to convert WAV → tone array
2. Generated header goes in `lib/AudioControl/sounds/`
3. Include in `AllSoundsTones.h`
4. Call play function in main.cpp

### DFPlayer Audio (`lib/AudioControlDFPlayer/`)

**Purpose**: Play MP3 files from SD card via DFPlayer Mini

**Key Functions**:
```cpp
void setup_dfplayer();
bool dfplayer_is_ready();
void play_sound_file(uint8_t file_number);
void set_volume(uint8_t volume);  // 0-30
```

**File Organization**:
- Files on SD card: `0001_name.mp3`, `0002_name.mp3`, etc.
- Track numbers map to constants in `AudioControlDFPlayer.h`
- Use `tools/prepare_sd_card.ps1` to set up SD card

### IR Control (`lib/IRControl/`)

**Purpose**: Detect MagicBand IR wands using custom protocol

**Key Functions**:
```cpp
void setup_ir();
uint32_t loop_ir();  // Returns wand_id or 0
```

**Protocol**:
- Uses IRremote library with custom decoder
- 64-bit MagicBand protocol
- Extracts wand_id from payload
- Returns 0 if no wand or decode error

### RFID Control (`lib/RFIDControl/`)

**Purpose**: Detect RFID bands/cards using RC522 reader

**Key Functions**:
```cpp
void setup_rfid();
uint32_t loop_rfid();  // Returns band_id (UID) or 0
void rfid_diagnostic();  // Health check
```

**Hardware**:
- Uses MFRC522 library
- SPI communication (GPIO 5, 18, 19, 22, 23)
- Reads 4-byte UID from Mifare cards
- 3.3V only (not 5V!)

**Card Detection**:
1. Check for card present
2. Read card serial (UID)
3. Convert UID bytes to uint32_t
4. Return for processing

### Servo Control (`lib/ServoControl/`)

**Purpose**: Control lid/door servo motor

**Key Functions**:
```cpp
void setup_servo();
void open_lid();
void close_lid();
void toggle_lid();
bool is_lid_open();
```

**Timing**:
- Slow movement (15ms per degree)
- Auto-close after 15 seconds
- LED fade on close

### Home Assistant Control (`lib/HomeAssistantControl/`)

**Purpose**: WiFi/MQTT integration with Home Assistant

**Key Functions**:
```cpp
void setup_home_assistant();
void loop_home_assistant();
bool is_system_enabled();
uint8_t get_ha_brightness();
unsigned long get_ha_cooldown();
void publish_wand_activation(uint32_t id);
```

**Configuration** (`HomeAssistantControl.h`):
```cpp
#define WIFI_SSID "your_network"
#define WIFI_PASSWORD "your_password"
#define MQTT_SERVER "192.168.1.100"
#define MQTT_USER "username"
#define MQTT_PASSWORD "password"
```

**Features**:
- Automatic Home Assistant discovery
- Remote enable/disable
- Brightness control
- Cooldown adjustment
- Wand activation notifications

### OTA Control (`lib/OTAControl/`)

**Purpose**: Over-the-air firmware updates

**Key Functions**:
```cpp
void setup_ota();
void loop_ota();  // Must be called frequently
```

**Visual Feedback**:
- White LEDs: OTA starting
- Blue pulsing: Update in progress
- Green flash: Success
- Red flash: Error

**Configuration**:
```cpp
#define OTA_HOSTNAME "MagicBand.local"
#define OTA_PASSWORD "magicband2025"
#define OTA_PORT 3232
```

## Data Flow

### Input Detection Flow

#### IR Version:
```
IR LED → IR Receiver (GPIO14)
    ↓
IRremote Library
    ↓
decodeMagicBand()
    ↓
Extract wand_id (uint32_t)
    ↓
Return to main.cpp
```

#### RFID Version:
```
RFID Card → RC522 Reader (SPI)
    ↓
MFRC522 Library
    ↓
Read UID bytes
    ↓
Convert to uint32_t
    ↓
Return to main.cpp
```

### Effect Trigger Flow

```
main.cpp receives ID
    ↓
Check cooldown
    ↓
Search BAND_CONFIGS array
    ↓
Match found?
    ├─ Yes → Trigger effects
    │         ├─ set_color(led_color)
    │         ├─ delay(50ms)  // Power settle
    │         ├─ play_sound()
    │         └─ toggle_servo()
    │
    └─ No → Show error feedback
              ├─ flash_color(blue)
              └─ play_error_sound()
```

## Adding New Features

### Adding a New Character/Band

1. **Define ID** (`lib/IRControl/IRControl.h` or `lib/RFIDControl/RFIDControl.h`):
```cpp
#define WAND_NEWCHAR 12345  // IR version
// or
#define BAND_NEWCHAR 0xABCD1234  // RFID version
```

2. **Add to Configuration** (`src/main.cpp`):
```cpp
BandConfig BAND_CONFIGS[] = {
    // Existing configs...
    {
        BAND_NEWCHAR,
        CRGB::Orange,  // LED color
        {SOUND_SPARKLE, SOUND_WAND, SOUND_CHIMES},  // Sound rotation
        3,  // Number of sounds
        0   // Current index
    }
};
```

3. **Test**:
   - Upload firmware
   - Scan new wand/band
   - Verify color and sounds

### Adding a New LED Animation

1. **Create Function** (`lib/LEDControl/LEDControl.cpp`):
```cpp
void new_animation(CRGB color) {
    // Your animation code
    // Use non-blocking timing with millis()
    // Call FastLED.show() after changes
}
```

2. **Declare** (`lib/LEDControl/LEDControl.h`):
```cpp
void new_animation(CRGB color);
```

3. **Use in main.cpp**:
```cpp
new_animation(CRGB::Purple);
```

### Adding a New Sound

#### Tone-Based (Recommended):
```bash
# Convert WAV to tones
python tools/wav_to_tones.py input.wav output.h sound_name

# Include in lib/AudioControl/sounds/AllSoundsTones.h
#include "sound_name_tones.h"

# Call in main.cpp
play_sound_name_tones();
```

#### MP3-Based (DFPlayer):
```bash
# Convert to MP3
# tools/convert_audio_for_dfplayer.ps1

# Copy to SD card
# tools/prepare_sd_card.ps1 -DriveLetter F

# Add constant in AudioControlDFPlayer.h
#define SOUND_NEW 7

# Use in main.cpp
play_sound_file(SOUND_NEW);
```

### Adding Debug Output

1. **Use Debug Macros** (defined in `lib/DebugConfig/DebugConfig.h`):
```cpp
DEBUG_PRINT("Value: ");
DEBUG_PRINTLN(value);
DEBUG_PRINTF("Formatted: %d\n", value);
```

2. **Control with Flag**:
```cpp
// DebugConfig.h
#define DEBUG_ENABLED 1  // Set to 0 to disable all debug output
```

3. **Benefits**:
   - Can disable all debug output with one flag
   - Saves ~2KB flash when disabled
   - Faster execution without serial overhead

## Code Conventions

### Naming
- **Constants**: `UPPER_CASE_WITH_UNDERSCORES`
- **Functions**: `lowercase_with_underscores`
- **Variables**: `lowercase_with_underscores`
- **Global variables**: Prefix with component name when possible

### File Organization
```
ComponentName/
├── ComponentName.h    # Interface, constants, configuration
├── ComponentName.cpp  # Implementation
└── README.md          # Component-specific docs (optional)
```

### Header Guards
```cpp
#ifndef COMPONENT_NAME_H
#define COMPONENT_NAME_H

// ... code ...

#endif
```

### Comments
```cpp
// Brief comment for simple statements

/**
 * Longer explanation for complex functions
 * Parameters:
 *   - param1: Description
 * Returns: Description
 */
```

### Magic Numbers
**Don't do this**:
```cpp
delay(5000);  // What does 5000 mean?
```

**Do this**:
```cpp
const unsigned long COOLDOWN_PERIOD = 5000;  // 5 seconds
delay(COOLDOWN_PERIOD);
```

## Build & Deploy

### Local Build
```powershell
# Build for default environment (esp32dev)
pio run

# Build for specific environment
pio run -e esp32dev-ota

# Clean build
pio run --target clean
```

### Upload
```powershell
# USB upload
pio run --target upload

# OTA upload
pio run -e esp32dev-ota --target upload
# or
pio run --target upload --upload-port MagicBand.local
```

### Serial Monitor
```powershell
pio device monitor --baud 115200

# or use VS Code task
# Command Palette → Tasks: Run Task → PlatformIO Monitor
```

### Environment Configuration

**platformio.ini**:
```ini
[platformio]
default_envs = esp32dev  # Default build environment

[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    fastled/FastLED@^3.10.3
    # ... other dependencies

[env:esp32dev-ota]
# Same as esp32dev but with OTA upload
upload_protocol = espota
upload_port = MagicBand.local
upload_flags = --port=3232 --auth=magicband2025
```

## Debugging

### Serial Debug Output

**Enable/Disable** (`lib/DebugConfig/DebugConfig.h`):
```cpp
#define DEBUG_ENABLED 1  // 1 = on, 0 = off
```

**View Output**:
```powershell
pio device monitor --baud 115200
```

**Common Debug Messages**:
```
=== MagicBand Initializing ===
RFID RC522 reader initialized
Firmware Version: 0x92
WiFi connected! IP: 192.168.1.123
MQTT connected
RFID Band activated (ID: 0x27CB1805) - Sound variation: 1
```

### Component Health Checks

**RFID Diagnostic**:
```cpp
rfid_diagnostic();  // Check reader health
```

**DFPlayer Status**:
```cpp
if (!dfplayer_is_ready()) {
    DEBUG_PRINTLN("DFPlayer not ready!");
}
```

**Home Assistant Connection**:
```cpp
// Check serial output for:
WiFi connected! IP: 192.168.1.123
MQTT connected
Discovery configs published
```

### Common Issues

**Build Errors**:
- Check `platformio.ini` dependencies
- Run `pio run --target clean`
- Delete `.pio` folder and rebuild

**Upload Errors**:
- USB: Check COM port, verify driver
- OTA: Check WiFi connection, verify hostname/IP

**Component Not Working**:
- Check serial monitor for initialization messages
- Verify wiring connections
- Check power supply (sufficient current)
- Enable debug output for detailed logging

**Power Issues**:
- Brown-out detection triggering
- LEDs at max brightness (~1A)
- Use external 5V 2A+ power supply
- Add delay between LED and servo activation

### Memory Optimization

**Check Memory Usage**:
```
RAM:   [=         ]  11.2% (36724 bytes)
Flash: [====      ]  41.8% (548589 bytes)
```

**Reduce Flash Usage**:
- Disable debug output (`DEBUG_ENABLED 0`)
- Use tone-based audio instead of WAV/MP3
- Remove unused libraries from `lib_deps`

**Reduce RAM Usage**:
- Use `const` for read-only data
- Use `PROGMEM` for large constant arrays
- Minimize global variables

## Best Practices

### Power Management
- 50ms delay between LED activation and servo
- FastLED power management settings configured
- Consider external 5V power for high LED counts

### Timing
- Use `millis()` for non-blocking delays
- Avoid `delay()` in component loop functions
- Main loop runs at 100ms intervals

### Error Handling
- Check return values from library functions
- Provide graceful degradation (continue if component fails)
- Log errors to serial with DEBUG_PRINTLN

### Testing
- Test each component independently
- Use serial monitor to verify initialization
- Test with real hardware before deploying
- Check memory usage fits within limits

### Documentation
- Update README.md for user-facing changes
- Add comments for complex logic
- Update this guide for architectural changes
- Keep code examples current

## Resources

- **PlatformIO Docs**: https://docs.platformio.org/
- **ESP32 Arduino API**: https://docs.espressif.com/projects/arduino-esp32/
- **FastLED Wiki**: https://github.com/FastLED/FastLED/wiki
- **MFRC522 GitHub**: https://github.com/miguelbalboa/rfid
- **DFPlayer Wiki**: https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299
