/*
 * RFID UID Scanner - Standalone utility to discover your RFID card UIDs
 * 
 * This is a simple standalone sketch to help you discover the UIDs
 * of your RFID cards/bands before integrating them into the main project.
 * 
 * Upload this sketch first, scan your cards, note the UIDs, then
 * update RFIDControl.h with the discovered UIDs.
 * 
 * Hardware Setup:
 *   RC522 -> ESP32
 *   SDA   -> GPIO5
 *   SCK   -> GPIO18
 *   MOSI  -> GPIO23
 *   MISO  -> GPIO19
 *   IRQ   -> (not connected)
 *   GND   -> GND
 *   RST   -> GPIO22
 *   3.3V  -> 3.3V (NOT 5V!)
 */

#include <SPI.h>
#include <MFRC522.h>

#define RFID_SS_PIN   5
#define RFID_RST_PIN  22

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n==========================================");
  Serial.println("RFID UID Scanner - Discover Your Card UIDs");
  Serial.println("==========================================\n");
  
  SPI.begin();
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
  
  Serial.print("MFRC522 Firmware Version: 0x");
  byte version = rfid.PCD_ReadRegister(rfid.VersionReg);
  Serial.println(version, HEX);
  
  if (version == 0x00 || version == 0xFF) {
    Serial.println("\n⚠️  WARNING: RFID reader not detected!");
    Serial.println("Check wiring and power (3.3V only!)");
    Serial.println("Expected version: 0x91 or 0x92\n");
  } else {
    Serial.println("✓ RFID reader detected successfully!\n");
  }
  
  Serial.println("Instructions:");
  Serial.println("1. Place RFID card/band near reader (2-4cm)");
  Serial.println("2. Note the UID shown below");
  Serial.println("3. Copy the 'C++ Define' line to RFIDControl.h");
  Serial.println("4. Repeat for all cards/bands");
  Serial.println("\nWaiting for RFID cards...\n");
}

void loop() {
  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  
  // Verify if the NUID has been read
  if (!rfid.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  
  // Get card type
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║         RFID CARD DETECTED!            ║");
  Serial.println("╚════════════════════════════════════════╝");
  
  // Print card type
  Serial.print("Card Type: ");
  Serial.println(rfid.PICC_GetTypeName(piccType));
  
  // Print UID in different formats
  Serial.print("UID Bytes: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // Convert to uint32_t (using first 4 bytes)
  uint32_t uid_32 = 0;
  byte bytes_to_use = (rfid.uid.size < 4) ? rfid.uid.size : 4;
  for (byte i = 0; i < bytes_to_use; i++) {
    uid_32 = (uid_32 << 8) | rfid.uid.uidByte[i];
  }
  
  Serial.print("UID as uint32_t: ");
  Serial.print(uid_32);
  Serial.print(" (decimal) = 0x");
  Serial.print(uid_32, HEX);
  Serial.println(" (hex)");
  
  // Print ready-to-use C++ define statement
  Serial.println("\n>>> C++ Define for RFIDControl.h:");
  Serial.print(">>> #define BAND_X 0x");
  if (uid_32 < 0x10000000) Serial.print("0");
  if (uid_32 < 0x1000000) Serial.print("0");
  if (uid_32 < 0x100000) Serial.print("0");
  if (uid_32 < 0x10000) Serial.print("0");
  if (uid_32 < 0x1000) Serial.print("0");
  if (uid_32 < 0x100) Serial.print("0");
  if (uid_32 < 0x10) Serial.print("0");
  Serial.println(uid_32, HEX);
  
  // Check if this is a supported card type
  if (piccType == MFRC522::PICC_TYPE_MIFARE_MINI ||  
      piccType == MFRC522::PICC_TYPE_MIFARE_1K ||
      piccType == MFRC522::PICC_TYPE_MIFARE_4K ||
      piccType == MFRC522::PICC_TYPE_MIFARE_UL) {
    Serial.println("\n✓ This card type is SUPPORTED!");
  } else {
    Serial.println("\n⚠️  Warning: This card type may not be supported!");
    Serial.println("Recommended: Mifare Classic 1K or Ultralight");
  }
  
  Serial.println("\n----------------------------------------\n");
  
  // Halt PICC
  rfid.PICC_HaltA();
  
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  
  // Small delay to prevent multiple reads
  delay(1000);
}
