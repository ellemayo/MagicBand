/**
 * Simple I2C Scanner
 * 
 * Scans the I2C bus and reports all detected devices.
 * Use this to verify the PN532 is responding on the I2C bus.
 * 
 * Expected output: Device at address 0x24 (PN532)
 */

#include <Arduino.h>
#include <Wire.h>

// I2C pins for ESP32
#define I2C_SDA 21
#define I2C_SCL 22

void setup() {
  Serial.begin(115200);
  delay(2000);  // Give time for serial monitor to open
  
  Serial.println("\n╔════════════════════════════════════════════╗");
  Serial.println("║         ESP32 I2C Bus Scanner              ║");
  Serial.println("╚════════════════════════════════════════════╝\n");
  
  Serial.print("I2C Pins: SDA=GPIO");
  Serial.print(I2C_SDA);
  Serial.print(", SCL=GPIO");
  Serial.println(I2C_SCL);
  Serial.println();
  
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);  // 100kHz - slow and reliable
  
  Serial.println("I2C bus initialized");
  Serial.println("Starting scan...\n");
}

void loop() {
  byte error, address;
  int deviceCount = 0;
  
  Serial.println("┌─────────────────────────────────────┐");
  Serial.println("│     Scanning I2C Bus (0x01-0x7F)    │");
  Serial.println("└─────────────────────────────────────┘\n");
  
  for (address = 1; address < 127; address++) {
    // Try to communicate with device at this address
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      // Device found!
      deviceCount++;
      
      Serial.print("✓ Device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(" (");
      Serial.print(address);
      Serial.print(")");
      
      // Identify known devices
      if (address == 0x24) {
        Serial.println(" → PN532 NFC/RFID Reader");
      } else if (address == 0x48) {
        Serial.println(" → PN532 (Alternate Address)");
      } else if (address == 0x68) {
        Serial.println(" → MPU6050 / DS1307 RTC");
      } else if (address == 0x76 || address == 0x77) {
        Serial.println(" → BMP280 / BME280");
      } else if (address == 0x3C || address == 0x3D) {
        Serial.println(" → OLED Display");
      } else {
        Serial.println(" → Unknown Device");
      }
      
    } else if (error == 4) {
      Serial.print("✗ Error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" → Unknown error");
    }
    
    delay(10);  // Small delay between checks
  }
  
  Serial.println();
  
  if (deviceCount == 0) {
    Serial.println("╔════════════════════════════════════════════╗");
    Serial.println("║  ⚠️  NO I2C DEVICES FOUND!                 ║");
    Serial.println("╚════════════════════════════════════════════╝");
    Serial.println("\nTroubleshooting:");
    Serial.println("  1. Check wiring connections (SDA, SCL, VCC, GND)");
    Serial.println("  2. Verify power supply (3.3V or 5V)");
    Serial.println("  3. Check pull-up resistors (4.7kΩ on SDA/SCL)");
    Serial.println("  4. For PN532: Verify DIP switches (1=OFF, 2=ON for I2C)");
    Serial.println("  5. Power cycle the PN532 after changing DIP switches");
    Serial.println("  6. Try different wires or breadboard connections");
  } else {
    Serial.println("╔════════════════════════════════════════════╗");
    Serial.print("║  ✓ Found ");
    Serial.print(deviceCount);
    Serial.print(" I2C device");
    if (deviceCount > 1) Serial.print("s");
    Serial.println("                            ║");
    Serial.println("╚════════════════════════════════════════════╝");
    
    // Check specifically for PN532
    bool foundPN532 = false;
    for (address = 1; address < 127; address++) {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      if (error == 0 && (address == 0x24 || address == 0x48)) {
        foundPN532 = true;
        break;
      }
    }
    
    if (foundPN532) {
      Serial.println("\n✓ PN532 detected! You can now use the UID scanner.");
      Serial.println("  Run: pio run --target upload --environment uid-scanner");
    } else {
      Serial.println("\n⚠️  PN532 NOT found at expected addresses (0x24 or 0x48)");
      Serial.println("   Check PN532 DIP switches and power cycle the module.");
    }
  }
  
  Serial.println("\n⏳ Scanning again in 5 seconds...\n");
  delay(5000);  // Scan every 5 seconds
}
