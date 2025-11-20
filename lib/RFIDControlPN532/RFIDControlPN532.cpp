#include <RFIDControlPN532.h>
#include <DebugConfig.h>

// Include PN532 library based on selected communication mode
#ifdef PN532_USE_I2C
  #include <Wire.h>
  #include <Adafruit_PN532.h>

  #define PN532_SDA 21
  #define PN532_SCL 22
  
  // Create PN532 object - will use Wire after we initialize it
  Adafruit_PN532 nfc(-1, -1);  // IRQ and RESET not used in I2C mode
#endif

#ifdef PN532_USE_SPI
  #include <SPI.h>
  #include <Adafruit_PN532.h>
  Adafruit_PN532 nfc(PN532_SS_PIN);
#endif

// Define the global variable here (declared as extern in the header)
rfid_band_info current_band;

// ISO 15693 Detection (Magic Bands)
// 
// ⚠️ IMPORTANT LIMITATION:
// The Adafruit PN532 library does NOT support ISO 15693 (Magic Band protocol).
// While the PN532 chip itself supports ISO 15693, the library doesn't expose the 
// required firmware commands (InCommunicateThru, InListPassiveTarget with baud 0x0A).
//
// For Magic Band support, you have two options:
// 1. Use MIFARE/NFC wristbands instead (ISO 14443A) - RECOMMENDED
//    - Work perfectly with PN532
//    - Identical functionality for this project
//    - See: docs/MAGIC_BAND_COMPATIBILITY.md
//
// 2. Switch to Seeed or elechouse PN532 library (complex code rewrite)
//    - Has ISO 15693 support
//    - Different API structure
//    - More difficult integration
//
bool read_iso15693_uid(uint8_t *uid, uint8_t *uidLength) {
  // Not supported by Adafruit PN532 library
  DEBUG_PRINTLN("[PN532] ISO15693 not supported - use MIFARE wristbands instead");
  return false;
}

void setup_rfid() {
  Serial.println("[PN532] ========== RFID SETUP START ==========");
  Serial.println("[PN532] Initializing RFID reader...");
  
  #ifdef PN532_USE_I2C
    Serial.println("[PN532] PN532_USE_I2C is DEFINED - Using I2C mode");
    Serial.println("[PN532] Initializing I2C bus on SDA=GPIO21, SCL=GPIO22...");
    
    // Initialize I2C bus FIRST - before constructing PN532 object
    Wire.begin(PN532_SDA, PN532_SCL);
    Wire.setClock(400000);  // 400kHz - same as scanner
    delay(100);
    Serial.println("[PN532] I2C bus initialized");
  #endif
  
  #ifdef PN532_USE_SPI
    Serial.println("[PN532] Using SPI mode");
  #endif
  
  // Initialize PN532 - object now exists
  Serial.println("[PN532] Initializing PN532...");
  nfc.begin();
  
  // Get firmware version to verify communication
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    DEBUG_PRINTLN("[PN532] ⚠️  ERROR: PN532 board not found!");
    DEBUG_PRINTLN("[PN532] Check wiring and power (3.3V or 5V depending on module)");
    return;
  }
  
  // Print firmware version
  DEBUG_PRINT("[PN532] ✓ Found chip PN5"); 
  DEBUG_PRINTLN((versiondata >> 24) & 0xFF, HEX);
  DEBUG_PRINT("[PN532] Firmware ver. "); 
  DEBUG_PRINT((versiondata >> 16) & 0xFF, DEC); 
  DEBUG_PRINT('.'); 
  DEBUG_PRINTLN((versiondata >> 8) & 0xFF, DEC);
  
  // Configure board to read RFID tags
  nfc.SAMConfig();
  
  // Set passive activation retries (0xFF = retry forever, 0x00 = no retry)
  // Using 0xFF for maximum compatibility with various card types
  nfc.setPassiveActivationRetries(0xFF);
  
  DEBUG_PRINTLN("[PN532] RFID reader initialized");
  DEBUG_PRINTLN("[PN532] ✓ Supports: ISO 14443A (MIFARE/NFC wristbands)");
  DEBUG_PRINTLN("[PN532] ✗ Magic Bands NOT supported (see docs/MAGIC_BAND_COMPATIBILITY.md)");
  DEBUG_PRINTLN("[PN532] Scan RFID band to activate...");
}

uint32_t loop_rfid() {
  uint8_t uid[8] = {0};
  uint8_t uidLength;
  bool success = false;
  
  // Try ISO 14443A first (MIFARE cards - most common)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
  
  if (success) {
    current_band.protocol = PROTOCOL_ISO14443A;
    current_band.is_magic_band = false;
    DEBUG_PRINTLN("[PN532] ISO 14443A card detected (MIFARE)");
  } else {
    // Try ISO 15693 (Magic Bands) using custom implementation
    success = read_iso15693_uid(uid, &uidLength);
    
    if (success) {
      current_band.protocol = PROTOCOL_ISO15693;
      current_band.is_magic_band = true;
      DEBUG_PRINTLN("[PN532] ISO 15693 tag detected (Magic Band!)");
    } else {
      // No card detected
      return 0;
    }
  }
  
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

// Check if ANY RFID card is currently present (lightweight check)
bool is_rfid_card_present() {
  uint8_t uid[8];
  uint8_t uidLength;
  
  // Quick check for ISO 14443A
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 10)) {
    return true;
  }
  
  // Quick check for ISO 15693
  if (read_iso15693_uid(uid, &uidLength)) {
    return true;
  }
  
  return false;
}

// Try to read the card if present, returns UID or 0 if failed
uint32_t read_rfid_if_present() {
  return loop_rfid();
}

// Try to read the card if present, returns full 64-bit UID or 0 if failed
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
