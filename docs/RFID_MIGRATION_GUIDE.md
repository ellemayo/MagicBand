# Migration Guide: IR Wands to RFID Bands

This guide will help you transition your MagiQuest system from IR wand detection to RFID band detection.

## Overview of Changes

The new RFIDControl library follows the exact same architecture as IRControl, making the transition straightforward. The main differences are:

1. **Hardware**: RC522 RFID reader instead of IR receiver
2. **Detection Method**: Physical proximity (2-4cm) instead of line-of-sight IR
3. **Identifier Format**: RFID UID (hex format) instead of MagiQuest wand IDs
4. **Pin Usage**: Uses SPI pins instead of single GPIO pin

## Hardware Setup

### Required Components
- **RC522 RFID Reader Module** (also called MFRC522)
- **RFID Cards/Bands/Wristbands** (Mifare Classic 1K or Ultralight recommended)
- **Jumper wires** for connections

### Wiring Connections

Connect the RC522 module to your ESP32:

```
RC522 -> ESP32
---------------
SDA   -> GPIO5  (or any available GPIO - configurable)
SCK   -> GPIO18 (hardware SPI - DO NOT CHANGE)
MOSI  -> GPIO23 (hardware SPI - DO NOT CHANGE)
MISO  -> GPIO19 (hardware SPI - DO NOT CHANGE)
IRQ   -> (not connected)
GND   -> GND
RST   -> GPIO22 (or any available GPIO - configurable)
3.3V  -> 3.3V
```

**CRITICAL**: The RC522 is a 3.3V device. Do NOT connect to 5V!

### Pin Conflicts to Consider

The default RFID pins may conflict with existing components:
- **GPIO18**: Used by both SPI SCK (RFID) and Servo (default)
  - **Solution**: Move servo to different pin (e.g., GPIO21) and update `ServoControl.h`
  - The example already has servo on GPIO18, so if using hardware SPI, you'll need to change one

**Recommended Pin Configuration:**
```cpp
// In RFIDControl.h
#define RFID_SS_PIN   5   // SDA/SS
#define RFID_RST_PIN  22  // Reset

// In ServoControl.h (if needed)
#define SERVO_PIN 21      // Changed from 18 to avoid SPI conflict
```

## Software Migration Steps

### Step 1: Update platformio.ini
The MFRC522 library has already been added for you:
```ini
lib_deps = 
    ...
    miguelbalboa/MFRC522@^1.4.11
```

### Step 2: Get Your RFID UIDs

Before updating the code, you need to discover the UIDs of your RFID bands:

1. Upload the RFID example code (see Step 3)
2. Open Serial Monitor (115200 baud)
3. Scan each RFID band/card on the reader
4. Note the UID printed in hex format (e.g., `0x12AB34CD`)

Example Serial Output:
```
RFID Band detected - UID: 0x12AB34CD
UID Bytes: 12 AB 34 CD
RFID Type: MIFARE 1KB
```

### Step 3: Update RFIDControl.h with Your UIDs

Edit `lib/RFIDControl/RFIDControl.h`:
```cpp
// Replace these with your actual RFID UIDs from Step 2
#define BAND_1 0x12AB34CD  // Blue band (August)
#define BAND_2 0x23BC45DE  // Green band (Ophelia)
#define BAND_3 0x34CD56EF  // Purple band (Evalette)
```

### Step 4: Update main.cpp

You have two options:

#### Option A: Use the Example File (Easiest)
1. Backup your current `src/main.cpp`
2. Copy `src/main_rfid_example.cpp` to `src/main.cpp`
3. Verify the sound mappings match your preferences

#### Option B: Manual Migration (More Control)
Make the following changes to your existing `main.cpp`:

1. **Change the include:**
   ```cpp
   // Old:
   #include <IRControl.h>
   
   // New:
   #include <RFIDControl.h>
   ```

2. **Update setup() function:**
   ```cpp
   // Old:
   setup_ir();
   
   // New:
   setup_rfid();
   ```

3. **Update loop() function:**
   ```cpp
   // Old:
   uint32_t wand_id = loop_ir();
   
   // New:
   uint32_t band_id = loop_rfid();
   ```

4. **Rename WAND_CONFIGS to BAND_CONFIGS:**
   ```cpp
   // Old:
   struct WandConfig { ... };
   WandConfig WAND_CONFIGS[] = { ... };
   
   // New:
   struct BandConfig { ... };
   BandConfig BAND_CONFIGS[] = { ... };
   ```

5. **Update config references in loop():**
   ```cpp
   // Old:
   if (wand_id == WAND_CONFIGS[i].wand_id)
   
   // New:
   if (band_id == BAND_CONFIGS[i].band_id)
   ```

6. **Update debug messages (optional):**
   ```cpp
   // Old:
   DEBUG_PRINT("Wand activated (ID: ");
   
   // New:
   DEBUG_PRINT("RFID Band activated (ID: 0x");
   DEBUG_PRINT(band_id, HEX);  // Print in hex format
   ```

### Step 5: Build and Upload

```bash
# Build the project
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor --baud 115200
```

Or use VS Code tasks:
- **Build**: PlatformIO Build
- **Upload**: PlatformIO Upload (USB)
- **Monitor**: PlatformIO Monitor

### Step 6: Test Your System

1. Power on the system
2. Watch for "RFID RC522 reader initialized" in serial monitor
3. Verify firmware version shows "Firmware Version: 0x91" or "0x92"
4. Scan an RFID band within 2-4cm of the reader
5. Verify the system responds with lights, sound, and servo action

## Troubleshooting

### RFID Reader Not Detected
**Symptom**: No "RFID RC522 reader initialized" message or firmware version is 0x00

**Solutions**:
- Check all wiring connections
- Verify 3.3V power (NOT 5V!)
- Ensure SPI pins are correct (GPIO18=SCK, GPIO19=MISO, GPIO23=MOSI)
- Try different SS and RST pins if current ones conflict
- Check for loose connections or bad solder joints

### Bands Not Reading
**Symptom**: System boots but doesn't detect RFID bands

**Solutions**:
- Ensure band is within 2-4cm of reader antenna
- Try different RFID cards/bands (some cheap ones don't work well)
- Verify card type is supported (should be Mifare Classic or Ultralight)
- Check serial monitor for "Unsupported RFID type" messages
- Increase antenna gain (already maxed in code)

### Servo Conflict
**Symptom**: Servo doesn't work or RFID doesn't work

**Solutions**:
- Both use GPIO18 by default - change one of them
- Recommended: Move servo to GPIO21
- Update `SERVO_PIN` in `lib/ServoControl/ServoControl.h`

### Wrong UIDs Detected
**Symptom**: System detects band but doesn't recognize it

**Solutions**:
- Scan band and note the UID from serial monitor
- Update `BAND_1`, `BAND_2`, `BAND_3` in `RFIDControl.h` with actual UIDs
- UIDs should be in format `0x12AB34CD` (hex with 0x prefix)
- Rebuild and upload after changing UIDs

## Performance Considerations

### Read Speed
- **RFID**: ~50-100ms detection time
- **IR**: ~20ms detection time
- **Impact**: Minimal - 5-second cooldown masks the difference

### Read Range
- **RFID**: 2-4cm (physical proximity required)
- **IR**: Several meters (line-of-sight)
- **Impact**: More reliable (no missed reads from pointing issues), but requires touching/close approach

### Power Consumption
- **RFID Reader**: ~50mA continuous
- **IR Receiver**: ~5mA continuous
- **Impact**: Use good power supply (already needed for LEDs/servo)

## Reverting to IR (If Needed)

If you need to go back to IR wands:

1. In `main.cpp`, change:
   - `#include <RFIDControl.h>` → `#include <IRControl.h>`
   - `setup_rfid()` → `setup_ir()`
   - `loop_rfid()` → `loop_ir()`
   - `BAND_CONFIGS` → `WAND_CONFIGS`
   - `band_id` → `wand_id`

2. Rebuild and upload

3. The RFIDControl library stays in your project but won't be compiled

## Next Steps

After successful migration:
- Label your RFID bands with character names
- Consider adding more bands (just add more `BAND_X` definitions)
- Experiment with different sound variations for each band
- Update Home Assistant configuration to reflect RFID bands

## Benefits of RFID Over IR

1. **No Line-of-Sight Required**: Works as long as band is close
2. **No "Aiming" Issues**: Perfect for young kids
3. **Unique IDs**: Every RFID card is globally unique
4. **Future Expandability**: Can read/write data to RFID cards
5. **Wearable**: Wristbands are easy to wear vs holding wands
6. **Durability**: No delicate IR LEDs to break

## Additional Resources

- **MFRC522 Library**: https://github.com/miguelbalboa/rfid
- **ESP32 SPI Pins**: https://randomnerdtutorials.com/esp32-spi-communication-arduino/
- **Mifare Card Types**: https://www.nxp.com/products/rfid-nfc/mifare-hf/mifare-classic:MC_41863
