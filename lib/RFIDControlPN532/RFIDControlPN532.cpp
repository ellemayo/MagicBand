#include <RFIDControlPN532.h>
#include <DebugConfig.h>

// Include PN532 library based on selected communication mode
#ifdef PN532_USE_I2C
  #include <Wire.h>
  #include <Adafruit_PN532.h>
  Adafruit_PN532 nfc(PN532_IRQ_PIN, PN532_RESET_PIN);
#endif

#ifdef PN532_USE_SPI
  #include <SPI.h>
  #include <Adafruit_PN532.h>
  Adafruit_PN532 nfc(PN532_SS_PIN);
#endif

// Define the global variable here (declared as extern in the header)
rfid_band_info current_band;

void setup_rfid() {
  DEBUG_PRINTLN("[PN532] Initializing RFID reader...");
  
  #ifdef PN532_USE_I2C
    DEBUG_PRINTLN("[PN532] Using I2C mode");
  #endif
  
  #ifdef PN532_USE_SPI
    DEBUG_PRINTLN("[PN532] Using SPI mode");
  #endif
  
  // Initialize PN532
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
  DEBUG_PRINTLN("[PN532] Supports: ISO 14443A (MIFARE) & ISO 15693 (Magic Bands)");
  DEBUG_PRINTLN("[PN532] Scan RFID band to activate...");
}

uint32_t loop_rfid() {
  uint8_t uid[8] = {0};
  uint8_t uidLength;
  bool success = false;
  
  // Try ISO 14443A first (MIFARE cards - most common)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50);
  
  if (success) {
    current_band.protocol = PROTOCOL_ISO14443A;
    current_band.is_magic_band = false;
    DEBUG_PRINTLN("[PN532] ISO 14443A card detected (MIFARE)");
  } else {
    // Try ISO 15693 (Magic Bands)
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO15693, uid, &uidLength, 50);
    
    if (success) {
      current_band.protocol = PROTOCOL_ISO15693;
      current_band.is_magic_band = true;
      DEBUG_PRINTLN("[PN532] ISO 15693 card detected (Magic Band!)");
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
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO15693, uid, &uidLength, 10)) {
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
      return "ISO 14443A (MIFARE)";
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
