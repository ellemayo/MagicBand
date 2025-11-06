# RFIDControl Library

## Overview
This library provides RFID reading capabilities using the MFRC522 (RC522) RFID reader module. It follows the same architectural pattern as the IRControl library, providing a simple interface for detecting RFID bands/cards and extracting their unique identifiers.

## Hardware Setup

### RC522 RFID Reader Pinout (ESP32)
The RC522 uses SPI communication with the following pin connections:

| RC522 Pin | ESP32 Pin | Description |
|-----------|-----------|-------------|
| SDA (SS)  | GPIO5     | SPI Chip Select (configurable) |
| SCK       | GPIO18    | SPI Clock (hardware SPI) |
| MOSI      | GPIO23    | SPI Master Out Slave In (hardware SPI) |
| MISO      | GPIO19    | SPI Master In Slave Out (hardware SPI) |
| IRQ       | Not used  | Interrupt (optional, not implemented) |
| GND       | GND       | Ground |
| RST       | GPIO22    | Reset (configurable) |
| 3.3V      | 3.3V      | Power (DO NOT USE 5V!) |

**IMPORTANT**: The RC522 is a 3.3V device. Do not connect it to 5V or it will be damaged!

### Pin Configuration
Default pins are defined in `RFIDControl.h`:
- `RFID_SS_PIN`: GPIO5 (SDA/SS pin)
- `RFID_RST_PIN`: GPIO22 (Reset pin)

You can change these in the header file if they conflict with other components.

## Supported RFID Types
This library supports the following Mifare RFID card types:
- Mifare Mini
- Mifare Classic 1K (most common)
- Mifare Classic 4K
- Mifare Ultralight (commonly used for wristbands)

## Usage

### Basic Setup
```cpp
#include <RFIDControl.h>

void setup() {
    setup_rfid();  // Initialize RFID reader
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

### Adding New RFID Bands
1. Scan your RFID band/card and note the UID printed in the serial monitor
2. Add the UID to `RFIDControl.h`:
```cpp
#define BAND_1 0x12345678  // Replace with your actual UID
```

### Integration with Main System
The library returns `0` when no band is detected and the band's UID as a `uint32_t` when a band is scanned. This follows the same pattern as `IRControl::loop_ir()`.

## Architecture Details

### Data Structure
The `rfid_band` union provides multiple ways to access the UID:
- `uid_32`: 32-bit integer for easy comparison
- `uid_bytes[4]`: Byte array for library compatibility
- `bytes.byte0-3`: Individual byte access

### Library Dependencies
- **MFRC522**: The official MFRC522 library for Arduino/ESP32
  - Add to `platformio.ini`: `miguelbalboa/MFRC522 @ ^1.4.11`

### Performance Characteristics
- **Read Time**: ~50-100ms per RFID detection
- **Range**: 2-4cm (typical for RC522)
- **Multiple Reads**: Library prevents rapid re-reads of the same card

## Troubleshooting

### Reader Not Detected
- Check wiring connections
- Verify 3.3V power supply
- Check SPI pins match ESP32 hardware SPI (SCK=18, MISO=19, MOSI=23)
- View firmware version in serial output (should show 0x91 or 0x92)

### Cards Not Reading
- Ensure card is within 2-4cm of reader
- Try increasing antenna gain (already set to max in setup)
- Check for interference from other components
- Verify card type is supported (Mifare Classic/Ultralight)

### Unknown UIDs
- First scan will print the UID in serial monitor
- Copy the hex value (e.g., 0x12345678) to `RFIDControl.h`
- Different cards may have 4, 7, or 10-byte UIDs (library uses first 4 bytes)

## Differences from IRControl
While following the same architecture, RFID has some key differences:
- **Active vs Passive**: RFID requires the band to be in close proximity (2-4cm)
- **Read Speed**: RFID is slightly slower (~50-100ms vs ~20ms for IR)
- **Protocol**: Uses SPI instead of dedicated IR receiver pin
- **Power**: RFID reader draws more power (~50mA vs ~5mA for IR)

## Future Enhancements
- IRQ pin support for interrupt-driven operation
- Support for reading/writing RFID data blocks
- Multi-band simultaneous detection (anti-collision)
- Adjustable antenna gain via configuration
