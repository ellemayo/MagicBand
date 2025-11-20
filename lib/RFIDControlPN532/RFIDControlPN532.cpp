#include <RFIDControlPN532.h>
#include <DebugConfig.h>

// PN532 library configuration
#ifdef PN532_USE_I2C
  #include <Wire.h>
  #include <Adafruit_PN532.h>

  #define PN532_SDA 21
  #define PN532_SCL 22
  
  // Global PN532 object (required by Adafruit library design)
  Adafruit_PN532 nfc(-1, -1);  // IRQ and RESET pins not used in I2C mode
#endif

#ifdef PN532_USE_SPI
  #include <SPI.h>
  #include <Adafruit_PN532.h>
  
  #define PN532_SS_PIN 5
  Adafruit_PN532 nfc(PN532_SS_PIN);
#endif

// Global variable for current band information
rfid_band_info current_band;

// ISO 15693 not supported by Adafruit PN532 library
// Use MIFARE/NFC wristbands instead - see docs/MAGIC_BAND_COMPATIBILITY.md
bool read_iso15693_uid(uint8_t *uid, uint8_t *uidLength) {
  return false;
}

void setup_rfid() {
  Serial.println("[PN532] ========== RFID SETUP START ==========");
  
  #ifdef PN532_USE_I2C
    Serial.print("[PN532] Initializing I2C bus...");
    Wire.begin(PN532_SDA, PN532_SCL);
    Wire.setClock(400000);
    delay(100);
    Serial.println("[PN532] I2C bus initialized");
  #endif
  
  #ifdef PN532_USE_SPI
    Serial.println("[PN532] Using SPI mode");
  #endif
  
  Serial.print("[PN532] Initializing PN532...");
  nfc.begin();
  
  uint32_t versiondata = nfc.getFirmwareVersion();
  
  if (!versiondata) {
    Serial.println(" FAILED!");
    DEBUG_PRINTLN("[PN532] ⚠️  ERROR: PN532 board not responding!");
    DEBUG_PRINTLN("[PN532] System will continue WITHOUT RFID functionality");
    return;
  }
  Serial.println(" SUCCESS!");
  Serial.print("\n[PN532] ✓ Found chip PN5"); 
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("[PN532]   Firmware version: "); 
  Serial.print((versiondata >> 16) & 0xFF, DEC); 
  Serial.print('.'); 
  Serial.println((versiondata >> 8) & 0xFF, DEC);
  
  // Configure board to read RFID tags
  nfc.SAMConfig();
  
  DEBUG_PRINTLN("[PN532] ========== RFID SETUP COMPLETE ==========");
  DEBUG_PRINTLN("[PN532] ✓ Supports: ISO 14443A (MIFARE/NFC wristbands)");
  DEBUG_PRINTLN("[PN532] ✗ Magic Bands NOT supported (see docs/MAGIC_BAND_COMPATIBILITY.md)");
  DEBUG_PRINTLN("[PN532] Ready to scan RFID bands!");
}

uint32_t loop_rfid() {
  uint8_t uid[8] = {0};
  uint8_t uidLength;
  
  // Only ISO 14443A is supported (MIFARE/NFC cards)
  bool success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
  
  if (!success) {
    return 0;  // No card detected
  }
  
  current_band.protocol = PROTOCOL_ISO14443A;
  current_band.is_magic_band = false;
  DEBUG_PRINTLN("[PN532] ISO 14443A card detected (MIFARE)");

  
  // Store UID length
  current_band.uid_length = uidLength;
  
  // Store UID bytes
  for (uint8_t i = 0; i < 8; i++) {
    if (i < uidLength) {
      current_band.uid.uid_bytes[i] = uid[i];
    } else {
      current_band.uid.uid_bytes[i] = 0;
    }
  }
  
  // Convert to uint32_t for backward compatibility
  uint32_t band_id_32 = uid_to_uint32(uid, uidLength);
  current_band.uid.uid_32 = band_id_32;
  
  // Convert to uint64_t for full UID
  uint64_t band_id_64 = uid_to_uint64(uid, uidLength);
  current_band.uid.uid_64 = band_id_64;
  
  DEBUG_PRINT("[PN532] Card detected - UID: 0x");
  DEBUG_PRINTLN(band_id_64, HEX);
  DEBUG_PRINT("[PN532] UID Bytes (");
  DEBUG_PRINT(uidLength);
  DEBUG_PRINT("): ");
  for (uint8_t i = 0; i < uidLength; i++) {
    if (uid[i] < 0x10) DEBUG_PRINT("0");
    DEBUG_PRINT(uid[i], HEX);
    DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN();
  DEBUG_PRINT("[PN532] Protocol: ");
  DEBUG_PRINTLN(get_protocol_name(current_band.protocol));
  
  return band_id_32;
}

uint64_t loop_rfid_64() {
  // Call regular loop to do the detection
  loop_rfid();
  
  // Return the full 64-bit UID
  return current_band.uid.uid_64;
}

/**
 * Helper function to convert UID byte array to uint32_t
 * Takes the first 4 bytes of the UID and combines them into a single uint32_t
 * For backward compatibility with RC522-based systems
 */
uint32_t uid_to_uint32(uint8_t *uid_bytes, uint8_t size) {
  uint32_t result = 0;
  
  // Use up to 4 bytes (or fewer if UID is shorter)
  uint8_t bytes_to_use = (size < 4) ? size : 4;
  
  for (uint8_t i = 0; i < bytes_to_use; i++) {
    result = (result << 8) | uid_bytes[i];
  }
  
  return result;
}

/**
 * Helper function to convert UID byte array to uint64_t
 * Takes up to 8 bytes of the UID and combines them into a single uint64_t
 * Essential for Magic Bands which have 8-byte UIDs
 */
uint64_t uid_to_uint64(uint8_t *uid_bytes, uint8_t size) {
  uint64_t result = 0;
  
  // Use up to 8 bytes (or fewer if UID is shorter)
  uint8_t bytes_to_use = (size < 8) ? size : 8;
  
  for (uint8_t i = 0; i < bytes_to_use; i++) {
    result = (result << 8) | uid_bytes[i];
  }
  
  return result;
}

// Check if RFID card is present (100ms timeout for reliable detection)
bool is_rfid_card_present() {
  uint8_t uid[8];
  uint8_t uidLength;
  return nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
}

// Read card if present
uint32_t read_rfid_if_present() {
  return loop_rfid();
}

// Read card if present (64-bit UID)
uint64_t read_rfid_if_present_64() {
  return loop_rfid_64();
}

// Get human-readable protocol name
const char* get_protocol_name(RFIDProtocol protocol) {
  switch (protocol) {
    case PROTOCOL_ISO14443A:
      return "ISO 14443A (MIFARE/NFC)";
    case PROTOCOL_ISO15693:
      return "ISO 15693 (Magic Band)";
    default:
      return "Unknown";
  }
}

// Check if the last detected card was a Magic Band
bool is_magic_band_detected() {
  return current_band.is_magic_band;
}

// Check if RFID reader is initialized
bool is_rfid_initialized() {
  return (nfc.getFirmwareVersion() != 0);
}
