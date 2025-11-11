/*
 * PN532 UID Scanner - Discover Magic Band and RFID Card UIDs
 * 
 * This standalone utility helps you discover the UIDs of:
 * - Disney Magic Bands (ISO 15693 - 8-byte UIDs)
 * - MIFARE cards (ISO 14443A - 4-byte UIDs)
 * - Other NFC/RFID tags
 * 
 * Upload this sketch first, scan your cards/bands, note the UIDs, then
 * update your main project with the discovered UIDs.
 * 
 * Hardware Setup (I2C Mode - Recommended):
 *   PN532 -> ESP32
 *   VCC   -> 3.3V or 5V (check your module)
 *   GND   -> GND
 *   SDA   -> GPIO21 (ESP32 default I2C)
 *   SCL   -> GPIO22 (ESP32 default I2C)
 * 
 *   Module DIP Switches: Set to I2C mode (typically OFF-ON)
 * 
 * Hardware Setup (SPI Mode - Alternative):
 *   PN532 -> ESP32
 *   VCC   -> 3.3V or 5V
 *   GND   -> GND
 *   SCK   -> GPIO18
 *   MISO  -> GPIO19
 *   MOSI  -> GPIO23
 *   SS    -> GPIO5
 * 
 *   Module DIP Switches: Set to SPI mode (typically ON-OFF)
 */

// ===== CONFIGURATION - Choose your communication mode =====
#define USE_I2C_MODE    // Comment this out if using SPI
// #define USE_SPI_MODE    // Uncomment this if using SPI

// ===== INCLUDES =====
#ifdef USE_I2C_MODE
  #include <Wire.h>
  #include <Adafruit_PN532.h>
  
  #define PN532_IRQ   (-1)
  #define PN532_RESET (-1)
  Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
#endif

#ifdef USE_SPI_MODE
  #include <SPI.h>
  #include <Adafruit_PN532.h>
  
  #define PN532_SS    5
  Adafruit_PN532 nfc(PN532_SS);
#endif

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════════════════╗");
  Serial.println("║  PN532 UID Scanner - Magic Bands & RFID Cards     ║");
  Serial.println("╚════════════════════════════════════════════════════╝\n");
  
  #ifdef USE_I2C_MODE
    Serial.println("Communication Mode: I2C");
    Serial.println("Pins: SDA=GPIO21, SCL=GPIO22\n");
  #endif
  
  #ifdef USE_SPI_MODE
    Serial.println("Communication Mode: SPI");
    Serial.println("Pins: SS=GPIO5, SCK=GPIO18, MISO=GPIO19, MOSI=GPIO23\n");
  #endif
  
  // Initialize PN532
  Serial.print("Initializing PN532...");
  nfc.begin();
  
  // Get firmware version
  uint32_t versiondata = nfc.getFirmwareVersion();
  
  if (!versiondata) {
    Serial.println(" FAILED!");
    Serial.println("\n⚠️  ERROR: PN532 board not found!");
    Serial.println("\nTroubleshooting:");
    Serial.println("  1. Check all wiring connections");
    Serial.println("  2. Verify power (3.3V or 5V depending on module)");
    Serial.println("  3. Check DIP switches match communication mode");
    #ifdef USE_I2C_MODE
      Serial.println("  4. I2C mode selected - switches should be: OFF-ON");
    #endif
    #ifdef USE_SPI_MODE
      Serial.println("  4. SPI mode selected - switches should be: ON-OFF");
    #endif
    Serial.println("\nSystem halted.");
    while (1);  // Halt
  }
  
  // Print version info
  Serial.println(" SUCCESS!");
  Serial.print("\n✓ Found chip PN5"); 
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("  Firmware version: "); 
  Serial.print((versiondata >> 16) & 0xFF, DEC); 
  Serial.print('.'); 
  Serial.println((versiondata >> 8) & 0xFF, DEC);
  
  // Configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("\n✓ PN532 configured successfully!");
  Serial.println("\n┌─────────────────────────────────────────────────┐");
  Serial.println("│ Supported Card Types:                           │");
  Serial.println("│  • Disney Magic Bands (ISO 15693)               │");
  Serial.println("│  • MIFARE Classic 1K/4K (ISO 14443A)            │");
  Serial.println("│  • MIFARE Ultralight (ISO 14443A)               │");
  Serial.println("│  • NTAG213/215/216 (ISO 14443A)                 │");
  Serial.println("│  • Most NFC tags                                │");
  Serial.println("└─────────────────────────────────────────────────┘");
  
  Serial.println("\n📝 Instructions:");
  Serial.println("  1. Place Magic Band or RFID card near reader (3-7cm)");
  Serial.println("  2. Note the UID and protocol shown below");
  Serial.println("  3. Copy the 'C++ Define' line to your project");
  Serial.println("  4. Repeat for all bands/cards");
  Serial.println("\n⏳ Waiting for cards/bands...\n");
}

// ===== MAIN LOOP =====
void loop() {
  uint8_t uid[8] = {0};
  uint8_t uidLength;
  bool success = false;
  const char* protocolName = "";
  bool isMagicBand = false;
  
  // Try ISO 14443A first (MIFARE cards - most common)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
  
  if (success) {
    protocolName = "ISO 14443A (MIFARE)";
    isMagicBand = false;
  } else {
    // Try ISO 15693 (Magic Bands)
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO15693, uid, &uidLength, 100);
    
    if (success) {
      protocolName = "ISO 15693 (Magic Band!)";
      isMagicBand = true;
    }
  }
  
  if (!success) {
    delay(100);
    return;
  }
  
  // ===== CARD DETECTED - DISPLAY INFORMATION =====
  Serial.println("╔════════════════════════════════════════════════════╗");
  if (isMagicBand) {
    Serial.println("║       ✨ DISNEY MAGIC BAND DETECTED! ✨            ║");
  } else {
    Serial.println("║           RFID CARD DETECTED!                      ║");
  }
  Serial.println("╚════════════════════════════════════════════════════╝");
  
  // Display protocol type
  Serial.print("Protocol: ");
  Serial.println(protocolName);
  
  // Display UID length
  Serial.print("UID Length: ");
  Serial.print(uidLength);
  Serial.println(" bytes");
  
  // Display UID in hex format (space-separated bytes)
  Serial.print("UID Bytes: ");
  for (uint8_t i = 0; i < uidLength; i++) {
    if (uid[i] < 0x10) Serial.print("0");
    Serial.print(uid[i], HEX);
    if (i < uidLength - 1) Serial.print(" ");
  }
  Serial.println();
  
  // Convert to numeric formats
  if (uidLength <= 4) {
    // 4-byte UID - display as uint32_t
    uint32_t uid_32 = 0;
    for (uint8_t i = 0; i < uidLength; i++) {
      uid_32 = (uid_32 << 8) | uid[i];
    }
    
    Serial.print("UID as uint32_t: ");
    Serial.print(uid_32);
    Serial.print(" (decimal) = 0x");
    if (uid_32 < 0x10000000) Serial.print("0");
    if (uid_32 < 0x1000000) Serial.print("0");
    if (uid_32 < 0x100000) Serial.print("0");
    if (uid_32 < 0x10000) Serial.print("0");
    if (uid_32 < 0x1000) Serial.print("0");
    if (uid_32 < 0x100) Serial.print("0");
    if (uid_32 < 0x10) Serial.print("0");
    Serial.print(uid_32, HEX);
    Serial.println(" (hex)");
    
    // Print ready-to-use C++ define
    Serial.println("\n>>> C++ Define (32-bit - for MIFARE compatibility):");
    Serial.print(">>> #define BAND_NAME 0x");
    for (int i = 7; i >= 0; i--) {
      if ((uid_32 >> (i * 4)) < 0x10 && i > 0) Serial.print("0");
    }
    Serial.println(uid_32, HEX);
    
  } else {
    // 5+ byte UID - display as uint64_t (for Magic Bands)
    uint64_t uid_64 = 0;
    uint8_t bytes_to_use = (uidLength < 8) ? uidLength : 8;
    
    for (uint8_t i = 0; i < bytes_to_use; i++) {
      uid_64 = (uid_64 << 8) | uid[i];
    }
    
    Serial.print("UID as uint64_t: ");
    Serial.print((uint32_t)(uid_64 >> 32), HEX);
    Serial.print((uint32_t)(uid_64 & 0xFFFFFFFF), HEX);
    Serial.println(" (hex)");
    
    // Also show as uint32_t (first 4 bytes) for backward compatibility
    uint32_t uid_32 = (uint32_t)(uid_64 >> ((bytes_to_use - 4) * 8));
    Serial.print("First 4 bytes as uint32_t: 0x");
    if (uid_32 < 0x10000000) Serial.print("0");
    if (uid_32 < 0x1000000) Serial.print("0");
    if (uid_32 < 0x100000) Serial.print("0");
    if (uid_32 < 0x10000) Serial.print("0");
    if (uid_32 < 0x1000) Serial.print("0");
    if (uid_32 < 0x100) Serial.print("0");
    if (uid_32 < 0x10) Serial.print("0");
    Serial.println(uid_32, HEX);
    
    // Print ready-to-use C++ defines
    Serial.println("\n>>> C++ Define (64-bit - RECOMMENDED for Magic Bands):");
    Serial.print(">>> #define BAND_NAME 0x");
    
    // Print leading zeros for proper formatting
    for (uint8_t i = 0; i < bytes_to_use; i++) {
      if (uid[i] < 0x10) Serial.print("0");
      Serial.print(uid[i], HEX);
    }
    Serial.println("ULL");
    
    Serial.println("\n>>> C++ Define (32-bit - for backward compatibility):");
    Serial.print(">>> #define BAND_NAME 0x");
    for (int i = 7; i >= 0; i--) {
      if ((uid_32 >> (i * 4)) < 0x10 && i > 0) Serial.print("0");
    }
    Serial.println(uid_32, HEX);
  }
  
  // Print recommendation
  if (isMagicBand) {
    Serial.println("\n✓ This is a MAGIC BAND (ISO 15693)!");
    Serial.println("  → Use the 64-bit define with 'ULL' suffix");
    Serial.println("  → Requires RFIDControlPN532 library");
    Serial.println("  → Use loop_rfid_64() for full UID support");
  } else {
    Serial.println("\n✓ This is a standard MIFARE card (ISO 14443A)");
    Serial.println("  → Works with both RC522 and PN532");
    Serial.println("  → Use 32-bit define for compatibility");
  }
  
  Serial.println("\n────────────────────────────────────────────────────\n");
  
  // Delay to prevent multiple reads of the same card
  delay(2000);
}
