# PN532 Hardware Troubleshooting Guide

## Error: I2C Communication Timeout (Error 263)

You're seeing `[E][Wire.cpp:513] requestFrom(): i2cRead returned Error 263` - this means the ESP32 can't communicate with the PN532 over I2C.

## Quick Checklist ✅

### 1. DIP Switch Configuration (MOST COMMON ISSUE)

The PN532 module has 2 DIP switches that select the communication mode:

**For I2C Mode (what we're using):**
```
Switch 1: OFF (or 0)
Switch 2: ON  (or 1)
```

**Visual Reference:**
```
┌────────┐
│ 1: OFF │  ← Switch 1 pushed to OFF/0 side
│ 2: ON  │  ← Switch 2 pushed to ON/1 side
└────────┘
```

**⚠️ IMPORTANT:** You must **power cycle** the PN532 after changing switches!
- Disconnect power (VCC)
- Wait 2 seconds
- Reconnect power
- The switches are only read during power-up

### 2. Wiring Connections

Double-check every connection:

```
PN532 Module    →    ESP32
────────────────────────────
VCC (3.3V/5V)   →    3.3V    ← Use 3.3V for most modules
GND             →    GND
SDA             →    GPIO21  ← I2C Data
SCL             →    GPIO22  ← I2C Clock
IRQ             →    Not connected (optional)
RSTPDN          →    Not connected (has pullup)
```

**Common Wiring Mistakes:**
- ❌ Swapped SDA and SCL
- ❌ Using 5V instead of 3.3V (can damage ESP32!)
- ❌ Loose connections in breadboard
- ❌ Wrong GPIO pins

### 3. Power Supply Issues

**PN532 Power Requirements:**
- Operating voltage: 3.3V or 5V (depends on your module)
- Current draw: ~100-150mA when active
- Most modules have built-in voltage regulators

**Check:**
- Is your PN532 module marked "3.3V" or "5V"?
- If using USB power, is it sufficient? (Try external 5V 2A supply)
- Are there any loose connections?

### 4. I2C Pull-up Resistors

I2C requires pull-up resistors on SDA and SCL lines. Most PN532 modules have these built-in, but some don't.

**Check your PN532 board:**
- Look for tiny resistors near SDA/SCL pins (usually marked R1, R2)
- Typical values: 4.7kΩ or 10kΩ

**If your module lacks pull-ups:**
- Add 4.7kΩ resistors from SDA to 3.3V
- Add 4.7kΩ resistors from SCL to 3.3V

### 5. I2C Address Conflict

The PN532 uses I2C address `0x24` by default. Check for conflicts:

```bash
# Flash the I2C scanner first
pio run --target upload --environment i2c-scanner
```

This will scan all I2C addresses and show what's detected.

## Step-by-Step Troubleshooting

### Step 1: Verify DIP Switches
1. Power off the PN532 (disconnect VCC)
2. Set switches: **1=OFF, 2=ON**
3. Take a photo to confirm
4. Reconnect power
5. Wait 3 seconds for module to initialize
6. Try the scanner again

### Step 2: Check Wiring with Multimeter
1. **Power off everything**
2. Use multimeter in continuity mode (beep mode)
3. Check each connection:
   - PN532 VCC pin → ESP32 3.3V pin (should beep)
   - PN532 GND pin → ESP32 GND pin (should beep)
   - PN532 SDA pin → ESP32 GPIO21 (should beep)
   - PN532 SCL pin → ESP32 GPIO22 (should beep)

### Step 3: Verify Power Supply
1. **Power on** the ESP32 + PN532
2. Measure voltage at PN532 VCC pin: should be 3.3V (±0.2V)
3. If voltage is low (<3.1V), try:
   - Different USB cable
   - Different power adapter
   - External 5V 2A power supply

### Step 4: Test with I2C Scanner
Flash the I2C scanner tool to verify I2C bus is working:

```bash
pio run --target upload --environment i2c-scanner
```

Expected output:
```
I2C Scanner
Scanning...
Found device at address 0x24 (PN532)
```

If you see:
- `No I2C devices found` → Wiring or power issue
- `0x24 detected` → PN532 is working! Problem is in scanner code
- Other addresses → Different device, check wiring

### Step 5: Try SPI Mode (Alternative)

If I2C just won't work, try SPI mode instead:

**DIP Switches for SPI:**
```
Switch 1: ON  (or 1)
Switch 2: OFF (or 0)
```

**SPI Wiring:**
```
PN532 Module    →    ESP32
────────────────────────────
VCC             →    3.3V
GND             →    GND
SCK             →    GPIO18 (SPI Clock)
MISO            →    GPIO19 (SPI MISO)
MOSI            →    GPIO23 (SPI MOSI)
SS (NSS)        →    GPIO5  (SPI Chip Select)
```

Then modify the scanner to use SPI - see `tools/pn532_uid_scanner.cpp` and uncomment the SPI sections.

## Common PN532 Module Variants

### Type 1: "PN532 NFC Module V3"
- Red PCB
- Has onboard voltage regulator (supports 3.3V-5V input)
- DIP switches on the board
- Built-in pull-up resistors
- **Use 3.3V or 5V VCC**

### Type 2: "PN532 Breakout Board"
- Blue/Green PCB
- DIP switches on side or bottom
- May need external pull-ups
- **Check board markings for voltage**

### Type 3: "Elechouse PN532"
- Black PCB
- DIP switches clearly labeled
- Built-in pull-ups
- **3.3V recommended**

## Testing Procedure

1. **Disconnect everything**
2. **Set DIP switches correctly**: 1=OFF, 2=ON
3. **Wire up in this order:**
   - GND first (always connect ground first!)
   - VCC (3.3V)
   - SDA → GPIO21
   - SCL → GPIO22
4. **Power on ESP32**
5. **Wait 3 seconds** for PN532 to initialize
6. **Upload scanner:** `pio run --target upload --environment uid-scanner`
7. **Open serial monitor:** `pio device monitor --baud 115200`

## Still Not Working?

### Check These:
- ✅ DIP switches set correctly AND power cycled
- ✅ Using 3.3V power (NOT 5V unless your module specifically requires it)
- ✅ SDA and SCL not swapped
- ✅ Good physical connections (wiggle wires - do errors change?)
- ✅ ESP32 GPIO pins work (try blinking an LED on GPIO21/22)
- ✅ PN532 module isn't damaged (try different module if possible)

### Try This Test Code

Upload this minimal test to verify I2C is working:

```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Wire.begin(21, 22);  // SDA=21, SCL=22
  Serial.println("I2C Scanner Starting...");
}

void loop() {
  byte error, address;
  int devices = 0;
  
  Serial.println("Scanning I2C bus...");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("Device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      devices++;
    }
  }
  
  if (devices == 0) {
    Serial.println("No I2C devices found!");
  }
  
  delay(5000);
}
```

Expected: Should see `Device found at 0x24` if PN532 is working.

## Last Resort: Hardware Issues

If nothing works after trying everything above:

1. **PN532 module might be defective**
   - Try a different PN532 module if available
   - Contact seller for replacement

2. **ESP32 I2C pins damaged**
   - Try different GPIO pins and update code
   - Try a different ESP32 board

3. **Incompatible module**
   - Some cheap clones have issues
   - Stick with Adafruit, Elechouse, or DFRobot brands

## Need More Help?

Post in GitHub issues with:
1. Photo of your wiring
2. Photo of DIP switch positions
3. PN532 module brand/model
4. Full serial output
5. Multimeter voltage readings

---

*Updated: November 17, 2025*
