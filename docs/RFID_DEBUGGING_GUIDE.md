# RFID Detection Debugging Guide

## Problem Description
RFID tags were not being detected in the main program, even though:
- ✅ The RFID scanner utility successfully read tags
- ✅ Hardware is confirmed working
- ✅ System boots properly

## Root Cause Analysis

### Primary Suspect: SPI Bus Conflicts
**FastLED Interference**: FastLED library uses bit-banging that disables interrupts briefly during `FastLED.show()`. This can interfere with SPI communication to the RFID reader.

**Original initialization order:**
1. `setup_leds()` - FastLED initialized
2. `fill_solid()` + `FastLED.show()` - **Interrupts disabled briefly**
3. `setup_rfid()` - SPI initialized ❌ **May fail silently**

### Secondary Issues
- **No health monitoring**: No way to know if RFID reader stays responsive
- **Timing conflicts**: 100ms main loop delay might be too long for reliable card detection
- **Component interference**: WiFi/MQTT/OTA may cause timing issues

## Fixes Implemented

### 1. Enhanced Debug Logging
**File**: `lib/RFIDControl/RFIDControl.cpp`

Added detailed debug output to `loop_rfid()`:
```cpp
if (!rfid.PICC_IsNewCardPresent()) {
    return 0;
}

DEBUG_PRINTLN("[RFID] Card detected - attempting to read...");

if (!rfid.PICC_ReadCardSerial()) {
    DEBUG_PRINTLN("[RFID] Failed to read card serial");
    return 0;
}

DEBUG_PRINTLN("[RFID] Card serial read successfully");
```

**What to look for**:
- If you see `[RFID] Card detected` → Reader is sensing cards
- If you see `[RFID] Failed to read card serial` → Communication problem
- If you see nothing → Reader not detecting cards at all

### 2. RFID Health Diagnostic Function
**File**: `lib/RFIDControl/RFIDControl.cpp` & `RFIDControl.h`

New function `rfid_diagnostic()` that:
- Checks firmware version (should be 0x91 or 0x92)
- Runs self-test to verify reader functionality
- Re-initializes reader after self-test
- Reports if reader is responding

**Called**:
- Once at startup (after `setup_rfid()`)
- Every 30 seconds in main loop

**What to look for**:
```
[RFID] === Diagnostic Check ===
[RFID] Firmware version: 0x92
[RFID] Reader is responding normally
[RFID] Self-test PASSED
[RFID] === End Diagnostic ===
```

**Bad output**:
```
[RFID] Firmware version: 0x00  ← Reader not responding!
[RFID] WARNING: Reader not responding! Check wiring.
[RFID] Self-test FAILED
```

### 3. Improved Initialization Timing
**File**: `src/main.cpp`

**Changes**:
- Added 100ms delay after `setup_rfid()` to let SPI stabilize
- Run diagnostic check immediately after initialization
- Periodic health checks every 30 seconds

```cpp
setup_rfid();

// Run RFID diagnostic to verify it's working
DEBUG_PRINTLN("\n=== RFID System Check ===");
delay(100);  // Let SPI stabilize
rfid_diagnostic();
DEBUG_PRINTLN("=========================\n");
```

### 4. Periodic Health Monitoring
**File**: `src/main.cpp`

Added constants:
```cpp
const unsigned long RFID_DIAGNOSTIC_INTERVAL = 30000;  // 30 seconds
unsigned long last_rfid_diagnostic = 0;
```

In `loop()`:
```cpp
// Periodic RFID health check (every 30 seconds)
if (current_time - last_rfid_diagnostic >= RFID_DIAGNOSTIC_INTERVAL) {
    DEBUG_PRINTLN("\n[Main] Running periodic RFID health check...");
    rfid_diagnostic();
    last_rfid_diagnostic = current_time;
}
```

## Testing Steps

### 1. Upload and Monitor
```bash
# Build and upload
pio run --target upload

# Open serial monitor
pio device monitor --baud 115200
```

### 2. Check Startup Output
Look for this sequence:
```
=== MagicBand (RFID) Initializing ===
RFID RC522 reader initialized
Firmware Version: 0x92

=== RFID System Check ===
[RFID] === Diagnostic Check ===
[RFID] Firmware version: 0x92
[RFID] Reader is responding normally
[RFID] Self-test PASSED
[RFID] === End Diagnostic ===
=========================

MagicBand RFID system ready!
```

### 3. Test Card Detection
Place RFID card near reader (2-4cm). You should see:
```
[RFID] Card detected - attempting to read...
[RFID] Card serial read successfully
RFID Band detected - UID: 0x27CB1805
UID Bytes: 27 CB 18 05
RFID Type: MIFARE 1KB
```

If recognized:
```
RFID Band activated (ID: 0x27CB1805) - Sound variation: 1
```

If not recognized:
```
Unknown RFID Band ID: 0x27CB1805
```

### 4. Monitor Periodic Health Checks
Every 30 seconds you should see:
```
[Main] Running periodic RFID health check...
[RFID] === Diagnostic Check ===
[RFID] Firmware version: 0x92
[RFID] Reader is responding normally
[RFID] Self-test PASSED
[RFID] === End Diagnostic ===
```

## Troubleshooting

### Reader Not Detecting Cards
**Symptom**: No `[RFID] Card detected` messages when scanning

**Checks**:
1. Verify wiring (especially SDA/SS on GPIO5)
2. Check power (3.3V only, NOT 5V!)
3. Try different cards (Mifare Classic 1K recommended)
4. Reduce distance (try 1-2cm instead of 4cm)
5. Check diagnostic shows firmware version 0x91 or 0x92

### Card Detected But Read Fails
**Symptom**: See `[RFID] Card detected` but then `Failed to read card serial`

**Checks**:
1. SPI wiring (SCK=18, MISO=19, MOSI=23)
2. Power supply stability (add capacitor near reader)
3. Disable WiFi temporarily to test:
   ```cpp
   // Comment out in setup():
   // setup_home_assistant();
   ```

### Reader Stops Working After Some Time
**Symptom**: Works at startup but stops detecting later

**Checks**:
1. Watch periodic diagnostic - does firmware version change to 0x00 or 0xFF?
2. Check for brown-out (power supply insufficient)
3. FastLED brightness too high (reduces available power)
4. WiFi transmit causing power drops

### Firmware Version Shows 0x00 or 0xFF
**Symptom**: Diagnostic shows `WARNING: Reader not responding!`

**This means**:
- Reader lost power
- SPI bus not working
- Wiring loose/disconnected
- Wrong GPIO pins

**Fix**:
1. Check all wiring with multimeter
2. Verify 3.3V power at reader
3. Try adding 10µF capacitor across power pins
4. Move reader away from LED strip (EMI interference)

## Hardware Considerations

### Power Budget
- ESP32: ~200mA
- WS2812B LEDs (17 @ max): ~1020mA
- RFID Reader: ~40mA
- DFPlayer: ~50mA
- **Total**: Up to 1310mA

**Recommendation**: Use external 5V power supply (2A minimum)

### Wiring Quality
- Keep RFID wiring SHORT (<10cm ideal)
- Use twisted pairs for SPI signals
- Add 10µF capacitor at RFID module
- Separate ground for LED power if possible

### EMI/Interference
- Keep RFID antenna away from LED strip
- Route RFID wires away from LED data line
- Add ferrite bead on RFID power if available

## Expected Behavior After Fix

### Startup Sequence
1. LEDs light up (blue)
2. RFID initialized
3. Diagnostic confirms reader working
4. WiFi connects
5. System ready message
6. Periodic health checks every 30s

### Normal Operation
1. Card placed near reader (2-4cm)
2. `[RFID] Card detected` message
3. Card serial read successfully
4. UID displayed
5. Band configuration matched
6. LEDs change color
7. Sound plays
8. Cooldown period begins

### Debug Output Volume
With all debug enabled, expect:
- **Startup**: ~30 lines
- **Each card scan**: ~6 lines
- **Periodic check**: ~4 lines every 30s
- **Home Assistant**: Various connection messages

## Performance Notes

### Detection Speed
- **Scanner utility**: ~50-100ms per scan
- **Main program**: ~100ms (limited by MAIN_LOOP_DELAY)
- Adding 100ms delay between scans is intentional to prevent multiple reads

### SPI Speed
- Default: 4MHz
- Can be increased but may cause issues with long wires
- Current setting is conservative for reliability

### Memory Usage
- RFID library: ~2KB
- Debug strings: ~1KB
- SPI buffers: ~512 bytes

## Next Steps

1. **Upload firmware** with diagnostic enhancements
2. **Monitor serial output** for diagnostic messages
3. **Test card detection** with known working cards
4. **Check periodic diagnostics** - reader should stay healthy
5. **If still failing**: Share full serial output for analysis

## Success Criteria

✅ Firmware version shows 0x91 or 0x92  
✅ Self-test passes  
✅ Card detection messages appear  
✅ UIDs are read correctly  
✅ Periodic diagnostics continue to pass  
✅ No firmware version drops to 0x00/0xFF  

When all these are true, RFID detection is working correctly!
