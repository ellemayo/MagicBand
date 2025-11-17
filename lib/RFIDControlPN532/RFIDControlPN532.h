#ifndef RFID_CONTROL_PN532_H
#define RFID_CONTROL_PN532_H

#include <Arduino.h>
// Forward declaration - full Adafruit_PN532 library included in .cpp file only

// PN532 Communication Mode Selection
// Uncomment ONE of these to select your wiring mode:
#define PN532_USE_I2C    // Recommended: Only 2 wires needed
// #define PN532_USE_SPI    // Alternative: Faster but more wires

// PN532 Pin Configuration
#ifdef PN532_USE_I2C
  // I2C Mode (Recommended - Simplest wiring)
  // ESP32 Default I2C Pins:
  // SDA -> GPIO21
  // SCL -> GPIO22
  // Note: PN532 must have I2C mode selected (switch positions on module)
  #define PN532_IRQ_PIN   -1  // Optional: GPIO for interrupt-driven reads
  #define PN532_RESET_PIN -1  // Optional: GPIO for hardware reset
#endif

#ifdef PN532_USE_SPI
  // SPI Mode (Compatible with RC522 wiring)
  // Hardware SPI Pins:
  // SCK  -> GPIO18
  // MISO -> GPIO19
  // MOSI -> GPIO23
  #define PN532_SS_PIN    5   // Chip Select (configurable)
  #define PN532_IRQ_PIN   -1  // Optional: GPIO for interrupt (not required)
  #define PN532_RESET_PIN -1  // Optional: GPIO for hardware reset
#endif

// RFID band data structure
// Supports both 4-byte (MIFARE) and 8-byte (ISO 15693/Magic Band) UIDs
union rfid_band {
  uint64_t uid_64;       // 64-bit representation for 8-byte UIDs (Magic Bands)
  uint32_t uid_32;       // 32-bit representation for backward compatibility
  uint8_t uid_bytes[8];  // Byte array for library compatibility (supports up to 8 bytes)
  struct {
    uint8_t byte0;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
    uint8_t byte4;
    uint8_t byte5;
    uint8_t byte6;
    uint8_t byte7;
  } bytes;
};

// RFID protocol types supported by PN532
enum RFIDProtocol {
  PROTOCOL_UNKNOWN = 0,
  PROTOCOL_ISO14443A = 1,  // MIFARE Classic, Ultralight, DESFire, NTAG (4-7 byte UID)
  PROTOCOL_ISO15693 = 2    // Magic Bands, NFC Type V (8-byte UID) - custom implementation
};

// Extended band info with protocol detection
struct rfid_band_info {
  rfid_band uid;           // The UID union
  uint8_t uid_length;      // Actual UID length (4, 7, 8, or 10 bytes)
  RFIDProtocol protocol;   // Detected protocol type
  bool is_magic_band;      // True if detected as Magic Band (ISO 15693)
};

// External declaration - actual definition is in RFIDControlPN532.cpp
extern rfid_band_info current_band;

// Function declarations - Same API as RFIDControl for easy swapping
void setup_rfid();
uint32_t loop_rfid();  // Returns 32-bit UID for backward compatibility
uint64_t loop_rfid_64();  // Returns full 64-bit UID for Magic Bands
bool is_rfid_card_present();
uint32_t read_rfid_if_present();
uint64_t read_rfid_if_present_64();  // Full 64-bit version

// Helper functions
uint32_t uid_to_uint32(uint8_t *uid_bytes, uint8_t size);
uint64_t uid_to_uint64(uint8_t *uid_bytes, uint8_t size);

// ISO 15693 support functions (custom implementation)
bool read_iso15693_uid(uint8_t *uid, uint8_t *uidLength);

// Protocol-specific functions
const char* get_protocol_name(RFIDProtocol protocol);
bool is_magic_band_detected();

#endif
