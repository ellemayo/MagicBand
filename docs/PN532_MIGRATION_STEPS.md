# PN532 Migration - Simple Steps

When your PN532 NFC/RFID reader arrives, follow these steps to migrate from RC522 to PN532 for Magic Band support.

---

## Hardware Setup

### 1. Wire the PN532 Module

**I2C Mode (Recommended - Only 2 wires!):**
```
PN532 -> ESP32
--------------
VCC -> 3.3V or 5V (check your module)
GND -> GND
SDA -> GPIO21
SCL -> GPIO22
```

**Important**: Set the PN532 DIP switches to I2C mode (typically: OFF-ON)

---

## Software Changes

### 2. Update `platformio.ini`

**In `[env:esp32dev]` section:**
```ini
lib_deps = 
	fastled/FastLED@^3.10.3
	z3t0/IRremote@^4.5.0
	knolleary/PubSubClient@^2.8
	bblanchon/ArduinoJson@^6.21.3
	dfrobot/DFRobotDFPlayerMini@^1.0.6
	miguelbalboa/MFRC522@^1.4.11              ; ← REMOVE THIS LINE
	adafruit/Adafruit PN532@^1.3.1            ; ← ADD THIS LINE
```

**In `[env:esp32dev-ota]` section:**
```ini
lib_deps = 
	fastled/FastLED@^3.10.3
	z3t0/IRremote@^4.5.0
	knolleary/PubSubClient@^2.8
	bblanchon/ArduinoJson@^6.21.3
	dfrobot/DFRobotDFPlayerMini@^1.0.6
	miguelbalboa/MFRC522@^1.4.11              ; ← REMOVE THIS LINE
	adafruit/Adafruit PN532@^1.3.1            ; ← ADD THIS LINE
```

**In `[env:uid-scanner]` section:**
```ini
build_src_filter = 
	-<*>
	; +<../tools/rfid_uid_scanner.cpp>      ; ← COMMENT THIS OUT
	+<../tools/pn532_uid_scanner.cpp>        ; ← UNCOMMENT THIS
lib_deps = 
	; miguelbalboa/MFRC522@^1.4.11          ; ← COMMENT THIS OUT
	adafruit/Adafruit PN532@^1.3.1           ; ← UNCOMMENT THIS
```

---

### 3. Update Your Main Code

**In your main source file (e.g., `src/main.cpp`):**

Change the include:
```cpp
// OLD:
#include <RFIDControl.h>

// NEW:
#include <RFIDControlPN532.h>
```

**That's it!** The API is identical:
- `setup_rfid()` still works
- `loop_rfid()` still works
- All your character handling code stays the same

**Optional - For full Magic Band UID support:**
```cpp
// If you want to use 64-bit UIDs:
uint64_t band_id = loop_rfid_64();  // Instead of loop_rfid()
```

---

### 4. Update Band IDs (After Scanning)

After you scan your Magic Bands with the PN532 scanner, update your band definitions:

**For 8-byte UIDs (Magic Bands):**
```cpp
// Example - replace with your actual UIDs from scanner
#define BAND_HARRY  0x04E1B2C2D24A80ULL  // Note: ULL suffix required!
#define BAND_ELSA   0x04F3C4D5E26B90ULL
```

**For 4-byte UIDs (MIFARE cards - still works):**
```cpp
#define BAND_1  0x12AB34CD  // No ULL needed for 32-bit
```

---

## Testing Steps

### 5. Scan Your Magic Bands First
```bash
# Upload the scanner
pio run --target upload --environment uid-scanner

# Open Serial Monitor and scan each Magic Band
# Copy the UIDs shown
```

### 6. Build and Upload Main Project
```bash
# Build
pio run

# Upload
pio run --target upload
```

### 7. Test
- Open Serial Monitor (115200 baud)
- Should see: `[PN532] ✓ Found chip PN532...`
- Scan a Magic Band
- Should see: `[PN532] ISO 15693 card detected (Magic Band!)`

---

## Communication Mode Setup

The library defaults to **I2C mode**. If you need SPI mode, edit:

**File**: `lib/RFIDControlPN532/RFIDControlPN532.h`

```cpp
// For I2C (default):
#define PN532_USE_I2C    // ← Uncomment this
// #define PN532_USE_SPI    // ← Comment this

// For SPI:
// #define PN532_USE_I2C    // ← Comment this
#define PN532_USE_SPI    // ← Uncomment this
```

---

## Quick Reference

| What to Change | Old (RC522) | New (PN532) |
|----------------|-------------|-------------|
| **Include** | `#include <RFIDControl.h>` | `#include <RFIDControlPN532.h>` |
| **Library** | `MFRC522@^1.4.11` | `Adafruit PN532@^1.3.1` |
| **Scanner** | `rfid_uid_scanner.cpp` | `pn532_uid_scanner.cpp` |
| **Wiring** | 7 wires (SPI only) | 4 wires (I2C) or 7 wires (SPI) |
| **Protocol** | ISO 14443A only | ISO 14443A + ISO 15693 |
| **Band UIDs** | 4 bytes (0x12AB34CD) | 8 bytes (0x04E1B2C2D24A80ULL) |

---

## What Stays the Same ✅

- All function names (`setup_rfid()`, `loop_rfid()`)
- All return types for basic functions
- Main loop structure
- Character handling logic
- Cooldown timers
- LED/Audio/Servo integration
- Everything else in your project!

---

## Troubleshooting

**"PN532 board not found"**
- Check wiring (especially SDA/SCL if using I2C)
- Verify DIP switches match communication mode
- Check power (3.3V or 5V depending on module)

**"No cards detected"**
- Magic Bands need to be within 3-7cm
- Try a standard MIFARE card first to test hardware
- Check Serial Monitor for protocol detection messages

**"Wrong protocol detected"**
- The library tries ISO 14443A first, then ISO 15693
- This is normal - the correct UID will still be returned

---

## Summary

1. ✅ Wire PN532 using I2C (2 wires) or SPI (7 wires)
2. ✅ Update `platformio.ini` - swap library dependency
3. ✅ Change `#include <RFIDControl.h>` to `#include <RFIDControlPN532.h>`
4. ✅ Scan Magic Bands with PN532 scanner tool
5. ✅ Update band ID definitions with ULL suffix for 8-byte UIDs
6. ✅ Build and upload - everything else stays the same!

**Total code changes needed: ~3 lines!**
