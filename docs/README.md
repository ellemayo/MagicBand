# Documentation Index

Welcome to the MagicBand ESP32 project documentation. This index will help you find the information you need.

## 📚 Getting Started

Start with the main README for an overview of the project:
- **[Main README](../README.md)** - Project overview, features, and basic setup

## 🔧 Setup Guides

### Audio Setup
- **[DFPlayer Setup Guide](../DFPLAYER_SETUP_GUIDE.md)** - Configure DFPlayer Mini MP3 module
  - SD card preparation
  - Audio file conversion
  - Wiring and troubleshooting

### IoT Integration
- **[Home Assistant Setup](../HOME_ASSISTANT_SETUP.md)** - WiFi/MQTT/Home Assistant integration
  - WiFi configuration
  - MQTT setup
  - Available controls and sensors
  - Example automations

### Wireless Updates
- **[OTA Usage Guide](OTA_USAGE.md)** - Over-the-air firmware updates
  - Upload firmware wirelessly
  - LED visual feedback
  - Troubleshooting OTA

## 🏷️ RFID System

### Migration to RFID
- **[RFID Migration Guide](RFID_MIGRATION_GUIDE.md)** - Complete guide to switching from IR to RFID
  - Hardware setup and wiring
  - Software migration steps
  - Visual/audio feedback implementation
  - Advanced debugging techniques
  - Troubleshooting common issues

### Quick Reference
- **[RFID Quick Reference](RFID_QUICK_REFERENCE.md)** - Visual diagrams and quick lookup
  - Pin wiring diagram
  - IR vs RFID comparison table
  - Common issues & solutions
  - 5-step getting started guide

## 👨‍💻 Development

### Architecture & Patterns
- **[Developer Guide](DEVELOPER_GUIDE.md)** - Architecture and development patterns
  - Component architecture
  - Library structure
  - Adding new features
  - Code conventions
  - Build and deployment

## 📁 Archive

Historical documentation and development summaries:
- **[archive/](archive/)** - Historical documentation
  - Feature implementation summaries
  - Bug fix changelogs
  - Previous integration guides

## 🆘 Getting Help

### Troubleshooting Resources

**Audio Issues:**
- Check DFPLAYER_SETUP_GUIDE.md troubleshooting section
- Verify SD card format (FAT32)
- Check serial monitor for DFPlayer initialization

**RFID Issues:**
- See RFID_MIGRATION_GUIDE.md debugging section
- Run `rfid_diagnostic()` function
- Check RFID_QUICK_REFERENCE.md for common issues

**WiFi/MQTT Issues:**
- See HOME_ASSISTANT_SETUP.md troubleshooting section
- Verify credentials in HomeAssistantControl.h
- Check serial monitor for connection status

**Build Issues:**
- Verify PlatformIO is installed
- Check platformio.ini for correct environment
- Run `pio run --target clean` then rebuild

## 🎯 Quick Navigation by Task

**I want to...**

- **Add new wand/band** → See README.md "Adding New Wand Characters"
- **Convert audio files** → See DFPLAYER_SETUP_GUIDE.md "Setup Steps"
- **Switch to RFID** → See RFID_MIGRATION_GUIDE.md
- **Enable Home Assistant** → See HOME_ASSISTANT_SETUP.md
- **Update wirelessly** → See OTA_USAGE.md
- **Understand the code** → See DEVELOPER_GUIDE.md
- **Fix RFID detection** → See RFID_MIGRATION_GUIDE.md "Advanced Debugging"
- **Customize LED effects** → See DEVELOPER_GUIDE.md "LED System"

## 📝 Contributing

When adding new features or fixing bugs:
1. Update relevant documentation
2. Add debug output where appropriate
3. Test thoroughly with serial monitor
4. Update README.md if user-facing changes

## 🔗 External Resources

- **PlatformIO**: https://platformio.org/
- **ESP32 Arduino Core**: https://github.com/espressif/arduino-esp32
- **FastLED Library**: https://github.com/FastLED/FastLED
- **MFRC522 Library**: https://github.com/miguelbalboa/rfid
- **DFPlayer Mini**: https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299
