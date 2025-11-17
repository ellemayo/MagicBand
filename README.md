# MagicBand ESP32 Interactive System

> An ESP32-based magical interactive system that responds to **Disney Magic Bands**, **RFID cards**, or **IR wands** with synchronized LED lighting, servo-controlled mechanical actions, and audio playback.

## ✅ Magic Band Support Confirmed!

**Great news:** Disney Magic Bands (MagicBand+ and MagicBand 2) work perfectly with the PN532 reader!

**Supported RFID Cards/Bands:**
- ✅ **Disney Magic Bands** (ISO 14443A, 7-byte UID) - **TESTED AND WORKING!**
- ✅ **MIFARE Classic 1K/4K** (ISO 14443A, 4-byte UID)
- ✅ **MIFARE Ultralight** (ISO 14443A, 4-byte UID)
- ✅ **NTAG213/215/216** (ISO 14443A, 7-byte UID)
- ✅ **NFC Wristbands** (13.56MHz, ISO 14443A compatible)

See **[docs/MAGIC_BAND_COMPATIBILITY.md](docs/MAGIC_BAND_COMPATIBILITY.md)** for details.

## ✨ Features

### 🎵 Dual Audio System
- **Tone-Based Audio**: Mathematical synthesis via ESP32 DAC (memory efficient)
- **MP3 Playback**: DFPlayer Mini module for high-quality audio from SD card
- **Sound Variations**: Each character has 3 unique sounds that rotate on activation

### 🏷️ Flexible Input Methods
- **RFID Bands**: Touch-based detection with PN532 reader (3-7cm range, MIFARE/NFC cards)
- **IR Wands**: Line-of-sight MagicBand protocol detection (several meters)
- Choose the input method that works best for your project!

### 💡 LED Visual Effects
- Character-specific colors on full LED strip
- Cooldown pulse animation
- Accelerating chase effects
- Success/fail feedback animations
- Smooth fade transitions

### 🏠 IoT Integration (Optional)
- WiFi/MQTT connectivity
- Home Assistant auto-discovery
- Remote control and monitoring
- Usage statistics tracking
- Automated scheduling

### 📡 Wireless Updates (Optional)
- Over-the-air (OTA) firmware updates
- No USB cable needed after initial setup
- LED visual feedback during updates

## 📖 Documentation

### Quick Start
- **[Setup Guide](#hardware-setup)** - Get started with hardware setup below
- **[Building & Uploading](#building-and-uploading)** - Compile and upload firmware

### Detailed Guides
- **[📋 Documentation Index](docs/README.md)** - Complete documentation navigation
- **[🎵 DFPlayer Setup](DFPLAYER_SETUP_GUIDE.md)** - Configure MP3 audio playback
- **[🏷️ RFID Compatibility](docs/MAGIC_BAND_COMPATIBILITY.md)** - **IMPORTANT:** RFID card/band compatibility
- **[🏷️ RFID Migration Guide](docs/RFID_MIGRATION_GUIDE.md)** - Switch from IR to RFID
- **[🏠 Home Assistant Setup](HOME_ASSISTANT_SETUP.md)** - Enable IoT features
- **[📡 OTA Updates](docs/OTA_USAGE.md)** - Wireless firmware updates
- **[👨‍💻 Developer Guide](docs/DEVELOPER_GUIDE.md)** - Architecture and coding patterns

## 🎮 Character System
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
Full IoT control and monitoring - see **[HOME_ASSISTANT_SETUP.md](HOME_ASSISTANT_SETUP.md)** for complete guide.

**Controls**: Enable/disable system, adjust LED brightness, set cooldown time  
**Sensors**: Last wand used, activation count, uptime, lid state  
**Automations**: Schedule on/off times, brightness based on time of day, notifications

## 🔧 Hardware Setup

### IR Wand Version (Default)
- IR Receiver → GPIO14
- LED Strip → GPIO3
- Servo Motor → GPIO18
- Audio Output → GPIO25 → LM386 Amplifier → Speaker

### RFID Band Version (Recommended)
- **PN532 NFC/RFID Reader** (I2C mode):
  - SDA → GPIO21 (I2C Data)
  - SCL → GPIO22 (I2C Clock)
  - 3.3V → 3.3V (**NOT 5V!**)
  - GND → GND
  - **DIP Switches**: OFF-ON (I2C mode)
- LED Strip (WS2812B) → GPIO13
- DFPlayer Mini:
  - TX → GPIO16 (ESP32 RX)
  - RX → GPIO17 (ESP32 TX)
- Audio Output → Amplifier → Speaker

**✅ Works with Disney Magic Bands!** Tested and confirmed with MagicBand+ / MagicBand 2.

**See [docs/RFID_MIGRATION_GUIDE.md](docs/RFID_MIGRATION_GUIDE.md) for complete RFID setup.**

### Power Requirements
- **50ms delay** between LED and servo activation prevents brown-out
- LED power budget: 17 LEDs × 60mA = ~1020mA at max brightness
- Use external 5V 2A+ power supply for reliable operation
- Audio DAC initialized to silence (128) to prevent startup noise

## 🚀 Building and Uploading

### Using PlatformIO CLI
```bash
# Build only
pio run

# Build and upload via USB
pio run --target upload

# Upload via OTA (after initial USB upload)
pio run --target upload --upload-port MagicBand.local
```

### Using VS Code
- **Build**: Use "PlatformIO Build" task from Command Palette
- **Upload**: Use "PlatformIO Upload (USB)" or "Upload (OTA)" task
- **Monitor**: Use "PlatformIO Monitor" task (115200 baud)

### First-Time Setup
1. Connect ESP32 via USB
2. Build and upload firmware
3. Open Serial Monitor to see initialization
4. Configure WiFi credentials (optional, see HOME_ASSISTANT_SETUP.md)
5. Future uploads can be done via OTA (see docs/OTA_USAGE.md)

## ⚙️ Configuration

### Adding New Characters/Bands

**Step 1**: Define ID in `lib/RFIDControl/RFIDControl.h` (or `IRControl.h`):
```cpp
#define BAND_NEWCHAR 0x12345678  // RFID UID
// or for IR:
#define WAND_NEWCHAR 12345
```

**Step 2**: Add to configuration in `src/main.cpp`:
```cpp
BandConfig BAND_CONFIGS[] = {
    // ... existing configs
    {
        BAND_NEWCHAR,
        CRGB::Orange,  // LED color
        {SOUND_SPARKLE, SOUND_WAND, SOUND_CHIMES},  // 3 sound variations
        3,   // Number of sounds
        0    // Current index
    }
};
```

### Converting Audio Files

#### For Tone-Based Audio:
```bash
python tools/wav_to_tones.py input.wav output.h sound_name
```

#### For DFPlayer MP3:
See **[DFPLAYER_SETUP_GUIDE.md](DFPLAYER_SETUP_GUIDE.md)** for complete instructions.

### Adjusting Settings

**LED Brightness** (`lib/LEDControl/LEDControl.h`):
```cpp
#define LED_DEFAULT_BRIGHTNESS 10  // 0-255
```

**Cooldown Period** (`src/main.cpp`):
```cpp
const unsigned long COOLDOWN_PERIOD = 5000;  // milliseconds
```

**Debug Output** (`lib/DebugConfig/DebugConfig.h`):
```cpp
#define DEBUG_ENABLED 1  // Set to 0 to disable (saves ~2KB flash)
```

## 🐛 Troubleshooting

### RFID Issues
- **Reader not detected**: Check 3.3V power, verify SPI wiring
- **Cards not reading**: Try 1-2cm distance, use Mifare Classic 1K cards
- **Intermittent detection**: Add 10µF capacitor at reader, move away from LEDs
- See **[docs/RFID_MIGRATION_GUIDE.md](docs/RFID_MIGRATION_GUIDE.md)** for detailed debugging

### Audio Issues
- **No sound**: Check DFPlayer wiring, verify SD card FAT32 format
- **Stuttering**: Use 44.1kHz MP3, constant bitrate, mono
- See **[DFPLAYER_SETUP_GUIDE.md](DFPLAYER_SETUP_GUIDE.md)** for troubleshooting

### Build Issues
- Run `pio run --target clean` then rebuild
- Verify `platformio.ini` default environment is set
- Check PlatformIO installation

### Power Issues
- Brown-out detection triggering: Use external 5V power supply
- LEDs dimming servo: Increase delay between LED and servo (50ms+)
- Random resets: Check power supply capacity (2A minimum)

## 📁 Project Structure

```
lib/                          # Component libraries
├── AudioControl/             # Tone-based audio (DAC)
├── AudioControlDFPlayer/     # MP3 playback (DFPlayer)
├── BandConfig/               # Character/band configuration
├── DebugConfig/              # Debug output control
├── HomeAssistantControl/     # WiFi/MQTT/HA integration
├── IRControl/                # IR wand detection
├── LEDControl/               # RGB LED control
├── OTAControl/               # Over-the-air updates
├── RFIDControl/              # RFID band detection
└── ServoControl/             # Servo motor control

src/
└── main.cpp                  # Main program logic

docs/                         # Documentation
├── README.md                 # Documentation index
├── DEVELOPER_GUIDE.md        # Architecture and development
├── RFID_MIGRATION_GUIDE.md   # RFID setup and migration
├── RFID_QUICK_REFERENCE.md   # Quick RFID reference
├── OTA_USAGE.md              # Wireless updates
└── archive/                  # Historical documentation

tools/                        # Utility scripts
├── wav_to_tones.py           # Convert WAV to tone arrays
├── convert_audio_for_dfplayer.ps1  # Convert to DFPlayer MP3
└── prepare_sd_card.ps1       # Prepare DFPlayer SD card
```

## 🤝 Contributing

When adding features or fixing bugs:
1. Follow existing code conventions (see docs/DEVELOPER_GUIDE.md)
2. Add debug output where appropriate
3. Update relevant documentation
4. Test with Serial Monitor
5. Check memory usage fits within limits

## 📚 Additional Resources

### Documentation
- **[Complete Documentation Index](docs/README.md)**
- **[Developer Guide](docs/DEVELOPER_GUIDE.md)** - Architecture, patterns, and best practices
- **[AI Coding Agent Instructions](.github/copilot-instructions.md)** - Project context for AI assistants

### External Links
- **PlatformIO**: https://platformio.org/
- **ESP32 Arduino**: https://github.com/espressif/arduino-esp32
- **FastLED Library**: https://github.com/FastLED/FastLED
- **MFRC522 Library**: https://github.com/miguelbalboa/rfid
- **DFPlayer Mini**: https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299

## 📝 Recent Updates

### October-November 2025
- ✅ Sound variation system (3 sounds per character)
- ✅ Home Assistant integration (WiFi/MQTT)
- ✅ RFID support as alternative to IR
- ✅ Over-the-air (OTA) updates
- ✅ Enhanced visual feedback animations
- ✅ Debug output system with conditional compilation
- ✅ Improved power spike protection
- ✅ Centralized documentation

See `docs/archive/` for detailed change summaries.

## 📄 License

This project is open source. See individual library licenses for dependencies.
