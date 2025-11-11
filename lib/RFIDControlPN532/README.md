# RFIDControlPN532 Library

## Overview
This library provides RFID/NFC reading capabilities using the **PN532 NFC/RFID controller module**. It supports multiple RFID protocols including **ISO 15693** which is used by **Disney Magic Bands**. The library follows the same architectural pattern as the original RFIDControl library, providing easy migration while supporting 8-byte UIDs.

**Key Advantage**: The PN532 can read **Disney Magic Bands** (ISO 15693) as well as standard MIFARE cards (ISO 14443A).

---

## Supported Card Types

### ISO 14443A (MIFARE) - 4-byte UIDs
- ✅ MIFARE Classic 1K/4K
- ✅ MIFARE Ultralight
- ✅ MIFARE DESFire
- ✅ NTAG213/215/216
- ✅ Most common RFID wristbands/cards

### ISO 15693 - 8-byte UIDs
- ✅ **Disney Magic Bands** (all versions)
- ✅ NFC Type V tags
- ✅ ICODE tags
- ✅ Other ISO 15693 compatible devices

---

## Hardware Setup

### PN532 Module Wiring

The PN532 supports **two communication modes**. Choose the one that works best for your project:

#### Option 1: I2C Mode (Recommended - Simplest)
```
PN532 -> ESP32
--------------
VCC -> 3.3V or 5V (check your module specs)
GND -> GND
SDA -> GPIO21 (ESP32 default I2C SDA)
SCL -> GPIO22 (ESP32 default I2C SCL)

Other Components (same as RC522 version):
------------------------------------------
LED Strip   -> GPIO13 (FastLED DATA_PIN)
DFPlayer TX -> GPIO16 (ESP32 Serial2 RX)
DFPlayer RX -> GPIO17 (ESP32 Serial2 TX)
```

**Module Configuration**: 
- Set PN532 DIP switches to I2C mode (typically switches OFF-ON, but check your module's documentation)
- Edit `RFIDControlPN532.h` and ensure `#define PN532_USE_I2C` is uncommented

#### Option 2: SPI Mode (Faster, More Wires)
```
PN532 -> ESP32
--------------
VCC  -> 3.3V or 5V (check your module specs)
GND  -> GND
SCK  -> GPIO18 (hardware SPI - DO NOT CHANGE)
MISO -> GPIO19 (hardware SPI - DO NOT CHANGE)
MOSI -> GPIO23 (hardware SPI - DO NOT CHANGE)
SS   -> GPIO5  (configurable)

Other Components:
-----------------
LED Strip   -> GPIO13 (FastLED DATA_PIN)
DFPlayer TX -> GPIO16 (ESP32 Serial2 RX)
DFPlayer RX -> GPIO17 (ESP32 Serial2 TX)
```

**Module Configuration**:
- Set PN532 DIP switches to SPI mode (typically switches ON-OFF)
- Edit `RFIDControlPN532.h` and uncomment `#define PN532_USE_SPI` (comment out I2C)

### Power Requirements
- **3.3V or 5V**: Check your PN532 module specifications (most accept both)
- Read range: **3-7cm** (better than RC522!)
- Current draw: ~50-150mA during active reading

---

## Software Setup

### 1. Add Library Dependency

Add to `platformio.ini`:
```ini
lib_deps = 
    adafruit/Adafruit PN532@^1.3.1
```

### 2. Choose Communication Mode

Edit `lib/RFIDControlPN532/RFIDControlPN532.h`:

For I2C (recommended):
```cpp
#define PN532_USE_I2C    // Uncomment this
// #define PN532_USE_SPI    // Comment this out
```

For SPI:
```cpp
// #define PN532_USE_I2C    // Comment this out
#define PN532_USE_SPI    // Uncomment this
```

### 3. Discover Your Card UIDs

Use the PN532 scanner tool to discover your Magic Band/card UIDs:

```bash
pio run --target upload --environment pn532-scanner
```

Open Serial Monitor at 115200 baud and scan your Magic Bands. Note the UIDs.

---

## Usage

### Basic Setup (Drop-in Replacement)

The API is **identical** to RFIDControl for easy migration:

```cpp
#include <RFIDControlPN532.h>

void setup() {
    setup_rfid();  // Initialize PN532 reader
}

void loop() {
    uint32_t band_id = loop_rfid();  // Check for RFID bands
    
    if (band_id != 0) {
        // Band detected! Do something based on band_id
        if (band_id == BAND_1) {
            // Handle band 1
        }
    }
}
```

### Advanced: Using Full 64-bit UIDs (Magic Bands)

For Magic Bands with 8-byte UIDs, use the 64-bit functions:

```cpp
#include <RFIDControlPN532.h>

void setup() {
    setup_rfid();
}

void loop() {
    uint64_t band_id = loop_rfid_64();  // Get full 64-bit UID
    
    if (band_id != 0) {
        // Magic Band detected!
        if (band_id == BAND_HARRY) {
            // Handle Harry's Magic Band
        }
        
        // Check if it's actually a Magic Band (ISO 15693)
        if (is_magic_band_detected()) {
            Serial.println("This is a real Disney Magic Band!");
        }
    }
}
```

### Defining Band IDs

#### For 4-byte UIDs (MIFARE cards):
```cpp
#define BAND_ELSA    0x12AB34CD      // Standard format
```

#### For 8-byte UIDs (Magic Bands):
```cpp
#define BAND_HARRY   0x04E1B2C2D24A80ULL  // Note: ULL suffix required!
#define BAND_ELSA    0x04F3C4D5E26B90ULL
```

The `ULL` suffix tells the compiler this is an unsigned 64-bit integer.

---

## API Reference

### Core Functions (Same as RFIDControl)

```cpp
void setup_rfid();
```
Initialize the PN532 reader. Call once in `setup()`.

```cpp
uint32_t loop_rfid();
```
Check for RFID cards and return 32-bit UID (first 4 bytes). Returns 0 if no card detected.

```cpp
bool is_rfid_card_present();
```
Quick check if any card is present. Doesn't read the full UID.

```cpp
uint32_t read_rfid_if_present();
```
Reads card if present, returns 32-bit UID or 0.

### Extended Functions (PN532-specific)

```cpp
uint64_t loop_rfid_64();
```
Check for RFID cards and return **full 64-bit UID**. Essential for Magic Bands!

```cpp
uint64_t read_rfid_if_present_64();
```
Reads card if present, returns full 64-bit UID or 0.

```cpp
bool is_magic_band_detected();
```
Returns `true` if the last detected card was ISO 15693 (Magic Band).

```cpp
const char* get_protocol_name(RFIDProtocol protocol);
```
Returns human-readable protocol name ("ISO 14443A" or "ISO 15693").

### Global Variables

```cpp
extern rfid_band_info current_band;
```

Access detailed information about the last scanned card:
- `current_band.uid.uid_64` - Full 64-bit UID
- `current_band.uid.uid_32` - First 4 bytes as 32-bit
- `current_band.uid_length` - Actual UID length (4, 7, 8, or 10 bytes)
- `current_band.protocol` - Protocol type (PROTOCOL_ISO14443A or PROTOCOL_ISO15693)
- `current_band.is_magic_band` - True if Magic Band detected

---

## Migration from RFIDControl (RC522)

### What Stays the Same ✅
- Function names: `setup_rfid()`, `loop_rfid()`
- Return types for basic functions (uint32_t)
- Overall code structure in main.cpp
- Cooldown logic and character handling

### What Changes 📝

#### 1. Library Include
```cpp
// OLD:
#include <RFIDControl.h>

// NEW:
#include <RFIDControlPN532.h>
```

#### 2. platformio.ini
```ini
# OLD:
miguelbalboa/MFRC522@^1.4.11

# NEW:
adafruit/Adafruit PN532@^1.3.1
```

#### 3. Pin Configuration
See wiring diagrams above - I2C mode only needs 2 pins!

#### 4. Band ID Definitions (for Magic Bands)
```cpp
// OLD (RC522 - 4 bytes):
#define BAND_1 0x12AB34CD

// NEW (PN532 - 8 bytes for Magic Bands):
#define BAND_HARRY 0x04E1B2C2D24A80ULL  // Note ULL suffix!
```

#### 5. Optional: Use 64-bit Functions
```cpp
// If you want full Magic Band UID support:
uint64_t band_id = loop_rfid_64();  // Instead of loop_rfid()
```

---

## Comparison: RC522 vs PN532

| Feature | RC522 | PN532 |
|---------|-------|-------|
| **Protocols** | ISO 14443A only | ISO 14443A + ISO 15693 + more |
| **Magic Band Support** | ❌ No | ✅ Yes |
| **UID Length** | 4 bytes typical | 4-10 bytes (8 for Magic Bands) |
| **Read Range** | 2-4cm | 3-7cm |
| **Communication** | SPI only | I2C or SPI |
| **Wiring Complexity** | 7 wires (SPI) | 4 wires (I2C) or 7 wires (SPI) |
| **Price** | ~$1-3 | ~$5-15 |
| **Library Size** | Smaller | Larger |

---

## Troubleshooting

### PN532 Not Detected
**Symptom**: `ERROR: PN532 board not found!`

**Solutions**:
- Check all wiring connections
- Verify power (3.3V or 5V depending on module)
- Ensure correct communication mode selected (I2C vs SPI)
- Check DIP switches on PN532 module match your mode
- For I2C: verify pull-up resistors present (usually built-in)
- Try different pins if using SPI mode

### Magic Bands Not Reading
**Symptom**: System boots but doesn't detect Magic Bands

**Solutions**:
- Ensure band is within 3-7cm of reader antenna
- Magic Bands need to be "activated" - try older Magic Band 1.0 first
- Check Serial Monitor - should show "ISO 15693 card detected"
- Some newer Magic Band+ may have different protocols
- Try with a standard MIFARE card first to verify hardware works

### Cards Read as Wrong Protocol
**Symptom**: Magic Band detected as MIFARE or vice versa

**Solutions**:
- The library tries ISO 14443A first, then ISO 15693
- This is normal behavior and shouldn't affect functionality
- Check `current_band.protocol` or use `is_magic_band_detected()`

### Slow Detection
**Symptom**: Takes 2-3 seconds to detect cards

**Solutions**:
- Reduce `setPassiveActivationRetries()` value in `setup_rfid()`
- Use SPI mode instead of I2C for faster communication
- Decrease timeout parameter in `readPassiveTargetID()` calls

---

## Example Code

See `tools/pn532_uid_scanner.cpp` for a complete standalone example that:
- Detects both MIFARE cards and Magic Bands
- Shows full UID in multiple formats
- Identifies protocol type
- Provides ready-to-use C++ defines

---

## Performance Notes

- **First detection**: ~100-150ms (protocol auto-detection)
- **Subsequent reads**: ~50-100ms (cached protocol)
- **I2C vs SPI**: SPI is ~2x faster but requires more wires
- **Power consumption**: Higher than RC522 but still very efficient

---

## Credits

Built using the excellent **Adafruit PN532 library** by Adafruit Industries.

---

## License

Compatible with the original MagicBand project license.
