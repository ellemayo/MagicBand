# RFID System Quick Reference

## Hardware Comparison: IR vs RFID

### IR Wand System (Original)
```
┌─────────────┐
│  IR Wand    │ → [point] → [IR Receiver GPIO14]
└─────────────┘
Detection: Line-of-sight, several meters range
ID Source: Wand transmits MagiQuest protocol
```

### RFID Band System (New)
```
┌──────────────┐
│  RFID Band   │ → [touch] → [RC522 Reader]
└──────────────┘              ↓ SPI
                        [ESP32 GPIO5,18,19,22,23]
Detection: Physical proximity, 2-4cm range
ID Source: Card's permanent UID chip
Audio: DFPlayer Mini on GPIO16/17 (UART)
LEDs: FastLED on GPIO13
```

## Pin Wiring Diagram

```
ESP32                          RC522 RFID Reader
┌──────────────────┐          ┌──────────────┐
│                  │          │              │
│  GPIO5  (SDA) ●──┼──────────┼● SDA         │
│  GPIO18 (SCK) ●──┼──────────┼● SCK         │
│  GPIO23 (MOSI)●──┼──────────┼● MOSI        │
│  GPIO19 (MISO)●──┼──────────┼● MISO        │
│  GPIO22 (RST) ●──┼──────────┼● RST         │
│                  │          │              │
│  3.3V         ●──┼──────────┼● 3.3V        │
│  GND          ●──┼──────────┼● GND         │
│                  │          │              │
│  GPIO13 (LED) ●──┼──→ LED Strip            │
│  GPIO16 (RX)  ●──┼──→ DFPlayer TX          │
│  GPIO17 (TX)  ●──┼──→ DFPlayer RX          │
└──────────────────┘          └──────────────┘

⚠️  IMPORTANT: RC522 is 3.3V ONLY! Do NOT use 5V!
⚠️  No servo used in this configuration
```

## Component Library Architecture

Both IR and RFID follow the same pattern:

```
lib/IRControl/               lib/RFIDControl/
├── IRControl.h              ├── RFIDControl.h
├── IRControl.cpp            ├── RFIDControl.cpp
└── (external: IRremote)     └── README.md
                             (external: MFRC522)

API Pattern (identical):
  void setup_ir()            void setup_rfid()
  uint32_t loop_ir()         uint32_t loop_rfid()
  Returns: 0 or wand_id      Returns: 0 or band_id
```

## Data Flow Comparison

### IR System
```
1. IR LED → IR Receiver
2. IRremote library decodes signal
3. Custom decodeMagiQuest() extracts wand_id
4. Return wand_id as uint32_t
5. main.cpp matches against WAND_1, WAND_2, WAND_3
```

### RFID System
```
1. RFID Card → RC522 Antenna
2. MFRC522 library reads UID via SPI
3. uid_to_uint32() converts bytes to uint32_t
4. Return band_id as uint32_t  
5. main.cpp matches against BAND_1, BAND_2, BAND_3
```

## Configuration Files

### IR Version
```cpp
// lib/IRControl/IRControl.h
#define IR_RECEIVE_PIN 14
#define WAND_1 20451
#define WAND_2 22171
#define WAND_3 22752
```

### RFID Version
```cpp
// lib/RFIDControl/RFIDControl.h
#define RFID_SS_PIN   5
#define RFID_RST_PIN  22
#define BAND_1 0x12345678  // Your actual UID
#define BAND_2 0x23456789  // Your actual UID
#define BAND_3 0x34567890  // Your actual UID
```

## Main.cpp Changes (Minimal!)

```cpp
// OLD (IR):                    // NEW (RFID):
#include <IRControl.h>          #include <RFIDControl.h>
setup_ir();                     setup_rfid();
uint32_t wand_id = loop_ir();   uint32_t band_id = loop_rfid();
if (wand_id == WAND_1)          if (band_id == BAND_1)
```

## Common Issues & Solutions

| Symptom | Cause | Solution |
|---------|-------|----------|
| "Firmware Version: 0x00" | Reader not detected | Check wiring, especially SPI pins |
| Cards not reading | Wrong card type | Use Mifare Classic 1K or Ultralight |
| System crashes on scan | 5V on 3.3V pin | **CRITICAL**: Use 3.3V only! |
| Unknown band ID | UID not configured | Scan card, note UID, update BAND_X |
| LEDs don't work | Wrong DATA_PIN | Verify GPIO13 in LEDControl.h |
| No audio | DFPlayer issue | Check GPIO16/17 wiring, SD card |

## Getting Started (5 Steps)

1. **Wire it up**: Follow pin diagram above (3.3V only!)
2. **Build**: Already configured in platformio.ini
3. **Upload**: Use example code or modify main.cpp
4. **Scan**: Serial monitor will show card UIDs
5. **Configure**: Update BAND_1/2/3 with your UIDs

## Performance Characteristics

| Metric | IR | RFID | Notes |
|--------|----|----|-------|
| Read Time | ~20ms | ~50-100ms | Both acceptable for 5s cooldown |
| Range | Several meters | 2-4cm | RFID requires proximity |
| Accuracy | Line-of-sight issues | 99%+ | RFID more reliable |
| Power | ~5mA | ~50mA | RFID uses more power |
| Wearability | Hold wand | Wear band | RFID better for kids |
| Durability | IR LED fragile | Very durable | RFID more robust |

## Why Choose RFID?

✅ **Better for young kids**: No aiming required, just touch  
✅ **More reliable**: No "missed reads" from bad aim  
✅ **Wearable**: Wristbands easier than carrying wands  
✅ **Unique IDs**: Every RFID card is globally unique  
✅ **Future expandability**: Can read/write data to cards  

## Why Keep IR?

✅ **Authentic MagiQuest experience**: Original wands work  
✅ **Distance**: Works from far away  
✅ **Lower power**: Battery-friendly  
✅ **Existing hardware**: If you already have wands  

## Recommended RFID Products

- **MFRC522 Module**: $1-3 on Amazon/AliExpress/eBay
- **Mifare Classic 1K Cards**: $0.20-0.50 each in bulk
- **RFID Wristbands**: $1-2 each (more kid-friendly)
- **RFID Keyfobs**: $0.50-1.00 each (durable option)

Look for: "13.56MHz RFID" or "NFC Mifare Classic"

## Next Steps

1. Read **docs/RFID_MIGRATION_GUIDE.md** for detailed instructions
2. Order RC522 module + wristbands/cards
3. Wire up following diagram above
4. Use `main_rfid_example.cpp` as starting point
5. Scan your cards to get UIDs
6. Update RFIDControl.h with your UIDs
7. Test and enjoy!
