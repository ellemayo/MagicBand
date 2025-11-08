#include <RFIDControl.h>
#include <MFRC522.h>  // Include the full MFRC522 library only in the .cpp file
#include <SPI.h>
#include <DebugConfig.h>

// Define the global variable here (declared as extern in the header)
rfid_band current_band;

// Create MFRC522 instance
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

void setup_rfid() {
  // Initialize SPI bus
  SPI.begin();
  
  // Initialize MFRC522 RFID reader
  rfid.PCD_Init();
  
  // Optional: Set gain for better read range (options: 18, 23, 33, 38, 43, 48 dB)
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
  
  DEBUG_PRINTLN("RFID RC522 reader initialized");
  DEBUG_PRINT("Firmware Version: 0x");
  DEBUG_PRINTLN(rfid.PCD_ReadRegister(rfid.VersionReg), HEX);
  DEBUG_PRINTLN("Scan RFID band to activate...");
}

uint32_t loop_rfid() {
  uint32_t band_id = 0;
  
  // Reset the loop if no new card present on the sensor/reader
  // This saves the entire process when idle
  if (!rfid.PICC_IsNewCardPresent()) {
    return 0;
  }
  
  DEBUG_PRINTLN("[RFID] Card detected - attempting to read...");
  
  // Verify if the NUID has been read
  if (!rfid.PICC_ReadCardSerial()) {
    DEBUG_PRINTLN("[RFID] Failed to read card serial");
    return 0;
  }
  
  DEBUG_PRINTLN("[RFID] Card serial read successfully");
  
  // Get the RFID type
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  
  // Check if it's a Mifare Classic or Ultralight card (most common for bands/wristbands)
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_UL) {
    DEBUG_PRINT("Unsupported RFID type: ");
    DEBUG_PRINTLN(rfid.PICC_GetTypeName(piccType));
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return 0;
  }
  
  // Convert UID to uint32_t for easy comparison
  band_id = uid_to_uint32(rfid.uid.uidByte, rfid.uid.size);
  
  // Store in global structure
  current_band.uid_32 = band_id;
  for (uint8_t i = 0; i < 4 && i < rfid.uid.size; i++) {
    current_band.uid_bytes[i] = rfid.uid.uidByte[i];
  }
  
  DEBUG_PRINT("RFID Band detected - UID: 0x");
  DEBUG_PRINTLN(band_id, HEX);
  DEBUG_PRINT("UID Bytes: ");
  for (uint8_t i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) DEBUG_PRINT("0");
    DEBUG_PRINT(rfid.uid.uidByte[i], HEX);
    DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN();
  DEBUG_PRINT("RFID Type: ");
  DEBUG_PRINTLN(rfid.PICC_GetTypeName(piccType));
  
  // Halt PICC
  rfid.PICC_HaltA();
  
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  
  return band_id;
}

/**
 * Helper function to convert UID byte array to uint32_t
 * Takes the first 4 bytes of the UID and combines them into a single uint32_t
 * Most RFID cards have 4-byte UIDs, but some have 7 or 10 bytes
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
 * Diagnostic function to test RFID reader health
 * Call this periodically to verify the reader is still responsive
 */
void rfid_diagnostic() {
  DEBUG_PRINTLN("[RFID] === Diagnostic Check ===");
  
  // Check firmware version
  byte version = rfid.PCD_ReadRegister(rfid.VersionReg);
  DEBUG_PRINT("[RFID] Firmware version: 0x");
  DEBUG_PRINTLN(version, HEX);
  
  if (version == 0x00 || version == 0xFF) {
    DEBUG_PRINTLN("[RFID] WARNING: Reader not responding! Check wiring.");
  } else {
    DEBUG_PRINTLN("[RFID] Reader is responding normally");
  }
  
  // Perform self-test
  bool selfTestResult = rfid.PCD_PerformSelfTest();
  if (selfTestResult) {
    DEBUG_PRINTLN("[RFID] Self-test PASSED");
  } else {
    DEBUG_PRINTLN("[RFID] Self-test FAILED - may need reset");
  }
  
  // Re-initialize after self-test (self-test leaves reader in bad state)
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
  
  DEBUG_PRINTLN("[RFID] === End Diagnostic ===");
}
