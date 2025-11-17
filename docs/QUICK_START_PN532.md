# Quick Start: PN532 + MIFARE/NFC Wristbands

## Shopping List 🛒

**What You Need to Buy:**
1. **NFC RFID Wristbands** (13.56MHz, ISO 14443A)
   - Search terms: "NTAG213 wristband" or "13.56MHz NFC bracelet"
   - Quantity: 5-10 wristbands (get extras)
   - Cost: ~$15-25 for a 10-pack
   - Colors: Multiple colors available

**What You Already Have:**
- ✅ PN532 NFC/RFID Reader
- ✅ ESP32
- ✅ LED strips, servos, audio components

## Wiring (5 Minutes) 🔌

**PN532 to ESP32 (I2C Mode):**
```
PN532     →  ESP32
SDA       →  GPIO21
SCL       →  GPIO22
VCC       →  3.3V
GND       →  GND
```

**Set DIP Switches on PN532:**
- Switch 1: OFF
- Switch 2: ON
- (This enables I2C mode)

## Setup Process (15 Minutes) ⚙️

### 1. Flash UID Scanner
```bash
cd /path/to/MagicBand
pio run --target upload --environment uid-scanner
```

### 2. Open Serial Monitor
```bash
pio device monitor --baud 115200
```

### 3. Scan Wristbands
- Hold each wristband near PN532 (3-7cm range)
- Write down the UID for each one
- Assign each UID to a person/character

Example output:
```
Card detected!
Protocol: ISO 14443A (MIFARE)
UID: 0x12345678
```

### 4. Update Band Definitions

Edit `lib/BandConfig/BandConfig.h`:
```cpp
// Define your band UIDs here
#define BAND_AUGUST   0x12345678UL   // Blue wristband - August
#define BAND_OPHELIA  0x23456789UL   // Green wristband - Ophelia
#define BAND_EVALETTE 0x34567890UL   // Purple wristband - Evalette
```

### 5. Update Main Code

Edit `src/main.cpp` to use your band definitions:
```cpp
switch (band_id) {
  case BAND_AUGUST:
    set_color(CRGB::Blue);
    play_character_sound(0);  // August's sound
    break;
    
  case BAND_OPHELIA:
    set_color(CRGB::Green);
    play_character_sound(1);  // Ophelia's sound
    break;
    
  case BAND_EVALETTE:
    set_color(CRGB::Purple);
    play_character_sound(2);  // Evalette's sound
    break;
}
```

### 6. Upload Main Firmware
```bash
pio run --target upload
```

### 7. Test!
- Hold wristband near PN532
- Watch LEDs light up
- Hear audio play
- See servo activate (if enabled)

## Troubleshooting 🔧

### "PN532 board not found"
- Check wiring connections
- Verify DIP switches: OFF-ON for I2C mode
- Try different 3.3V power source

### "No card detected"
- Hold wristband closer (3-5cm)
- Make sure wristband is 13.56MHz
- Try a different wristband

### "Wrong UID detected"
- UIDs are case-sensitive in code
- Use `UL` suffix for uint32_t: `0x12345678UL`
- Make sure you're using the UID from serial monitor

### LEDs don't light up
- Check LED strip power (5V, 2A+)
- Verify LED data pin connection (GPIO13)
- Try lowering brightness in code

## What About Magic Bands? 🏰

**Disney Magic Bands don't work with this setup.** They use ISO 15693 protocol which isn't supported by the Adafruit PN532 library.

**Good news:** MIFARE/NFC wristbands work EXACTLY the same way:
- ✅ Unique IDs for each person
- ✅ Touch-to-activate
- ✅ Waterproof and durable
- ✅ Come in many colors
- ✅ Kids love them!
- ✅ Actually cheaper than Magic Bands
- ✅ Better read range

See **[docs/MAGIC_BAND_COMPATIBILITY.md](MAGIC_BAND_COMPATIBILITY.md)** for details.

## Next Steps 🚀

1. **Order wristbands** (search "NTAG213 silicone wristband")
2. **Wire up PN532** (takes 5 minutes)
3. **Scan wristband UIDs** (takes 5 minutes)
4. **Update code with UIDs** (takes 5 minutes)
5. **Upload and enjoy!**

## Need Help? 📚

- **[docs/PN532_STATUS.md](PN532_STATUS.md)** - Current integration status
- **[docs/RFID_MIGRATION_GUIDE.md](RFID_MIGRATION_GUIDE.md)** - Complete PN532 guide
- **[docs/MAGIC_BAND_COMPATIBILITY.md](MAGIC_BAND_COMPATIBILITY.md)** - Why Magic Bands don't work
- **[README.md](../README.md)** - Main project documentation

---

**Total time from zero to working: ~30 minutes + shipping time for wristbands**

*Updated: November 17, 2025*
