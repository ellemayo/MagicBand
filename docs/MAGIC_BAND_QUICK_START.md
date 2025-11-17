# Quick Start: Using Your Magic Band UID

## Your Magic Band UID

From your scan:
```
UID Bytes: 04 5C 92 F2 87 68 80
UID as 64-bit: 0x045C92F2876880ULL
```

## Step 1: Add to BandConfig.h

Create or edit `lib/BandConfig/BandConfig.h`:

```cpp
#ifndef BAND_CONFIG_H
#define BAND_CONFIG_H

#include <stdint.h>

// Magic Band UIDs (7-byte ISO 14443A - use ULL suffix)
#define BAND_YOUR_NAME 0x045C92F2876880ULL  // Your Magic Band

// You can add more as you scan them:
// #define BAND_PERSON2 0x04XXXXXXXXXXXXULL
// #define BAND_PERSON3 0x04XXXXXXXXXXXXULL

// Test card (4-byte MIFARE - use UL suffix)
#define CARD_TEST 0xA4C0F705UL

#endif
```

## Step 2: Update main.cpp

Edit `src/main.cpp` to use 64-bit UIDs:

```cpp
#include <Arduino.h>
#include <RFIDControlPN532.h>
#include <LEDControl.h>
#include <AudioControl.h>
#include <BandConfig.h>

// Timing
unsigned long last_activation = 0;
const unsigned long COOLDOWN_PERIOD = 3000;  // 3 seconds between activations

void setup() {
  Serial.begin(115200);
  
  // Initialize components
  setup_rfid();
  setup_leds();
  setup_audio();
  
  Serial.println("Magic Band system ready!");
}

void loop() {
  // Read Magic Band (64-bit UID)
  uint64_t band_id = loop_rfid_64();
  
  // Check if a band was detected
  if (band_id == 0) {
    return;  // No band detected
  }
  
  // Check cooldown
  unsigned long current_time = millis();
  if (current_time - last_activation < COOLDOWN_PERIOD) {
    return;  // Still in cooldown
  }
  
  // Process the band
  Serial.print("Magic Band detected: 0x");
  Serial.println((unsigned long)(band_id >> 32), HEX);
  Serial.println((unsigned long)(band_id & 0xFFFFFFFF), HEX);
  
  // Trigger effects based on which band it is
  switch (band_id) {
    case BAND_YOUR_NAME:
      Serial.println("Welcome [Your Name]!");
      set_color(CRGB::Blue);           // Blue LEDs
      play_chimes_tones();             // Play sound
      // toggle_lid();                 // Optional: activate servo
      break;
      
    // Add more cases as you scan more bands:
    // case BAND_PERSON2:
    //   set_color(CRGB::Green);
    //   play_sparkle_tones();
    //   break;
    
    default:
      // Unknown band
      Serial.println("Unknown band detected");
      set_color(CRGB::Red);
      delay(500);
      clear_leds();
      break;
  }
  
  // Update last activation time
  last_activation = current_time;
  
  // Wait for band to be removed
  delay(1000);
}
```

## Step 3: Build and Upload

```bash
pio run --target upload
```

## Step 4: Test!

1. Hold your Magic Band near the PN532 (3-7cm range)
2. LEDs should turn blue
3. Chimes sound should play
4. Serial monitor shows "Welcome [Your Name]!"

## Adding More Magic Bands

For each additional Magic Band:

1. **Scan it:**
   ```bash
   pio run --target upload --environment uid-scanner
   pio device monitor --baud 115200
   ```

2. **Copy the UID** (the one with `ULL` suffix)

3. **Add to BandConfig.h:**
   ```cpp
   #define BAND_PERSON2 0x04XXXXXXXXXXXXULL  // Replace X with actual UID
   ```

4. **Add case in main.cpp:**
   ```cpp
   case BAND_PERSON2:
     set_color(CRGB::Green);  // Different color
     play_sparkle_tones();    // Different sound
     break;
   ```

## Mixing Magic Bands and MIFARE Cards

You can use both! Just be aware of UID size:

```cpp
// For 4-byte cards, use 32-bit reading:
uint32_t card_id = loop_rfid();  // Returns 32-bit UID

// For 7-byte Magic Bands, use 64-bit reading:
uint64_t band_id = loop_rfid_64();  // Returns 64-bit UID
```

Or use the unified approach:

```cpp
// Read with full info
loop_rfid_64();  // Stores in global current_band structure

// Check UID length
if (current_band.uid_length == 4) {
  // It's a MIFARE card
  uint32_t card_id = current_band.uid.uid_32;
} else if (current_band.uid_length == 7) {
  // It's a Magic Band
  uint64_t band_id = current_band.uid.uid_64;
}
```

## Troubleshooting

**"Band not detected":**
- Hold closer (3-5cm works best)
- Make sure DIP switches are correct (OFF-ON for I2C)
- Check wiring: SDA→21, SCL→22, VCC→3.3V, GND→GND

**"Wrong person's effect triggers":**
- UIDs are case-sensitive
- Make sure you copied the ULL suffix version
- Use the full 64-bit UID from the scanner

**"Triggers multiple times":**
- Increase COOLDOWN_PERIOD (currently 3000ms = 3 seconds)
- Add a "remove band" check before allowing next read

---

*Your Magic Band UID: `0x045C92F2876880ULL`*

*Updated: November 17, 2025*
