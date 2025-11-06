#ifndef RFID_CONTROL_H
#define RFID_CONTROL_H

#include <Arduino.h>
// Forward declaration - full MFRC522 library included in .cpp file only

// RFID Reader Pin Configuration (RC522)
// SPI Pins for ESP32:
// SCK  -> GPIO18
// MISO -> GPIO19
// MOSI -> GPIO23
// SDA/SS -> GPIO5 (configurable)
// RST -> GPIO22 (configurable)
#define RFID_SS_PIN   5
#define RFID_RST_PIN  22

// RFID Band IDs (UID stored as uint32_t for easy comparison)
// These will be extracted from the 4-byte UID of each RFID card/band
#define BAND_1 0x12345678  // Replace with actual UID from your RFID bands
#define BAND_2 0x23456789  // Replace with actual UID from your RFID bands
#define BAND_3 0x34567890  // Replace with actual UID from your RFID bands

// RFID band data structure
// Stores the UID in a format compatible with the RC522 library
union rfid_band {
  uint32_t uid_32;      // 32-bit representation for easy comparison
  uint8_t uid_bytes[4]; // Byte array for RC522 library compatibility
  struct {
    uint8_t byte0;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
  } bytes;
};

// External declaration - actual definition is in RFIDControl.cpp
extern rfid_band current_band;

// Function declarations
void setup_rfid();
uint32_t loop_rfid();

// Helper function to convert UID byte array to uint32_t
uint32_t uid_to_uint32(uint8_t *uid_bytes, uint8_t size);

#endif
