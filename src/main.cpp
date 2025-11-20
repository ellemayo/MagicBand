#include <Arduino.h>
#include <DebugConfig.h>

#include <BandConfig.h>
#include <RFIDControlPN532.h>
#include <LEDControl.h>
#include <AudioControlDFPlayer.h>
#include <HomeAssistantControl.h>
#include <OTAControl.h>

// Firmware version for tracking OTA updates
#define FIRMWARE_VERSION "1.0.0-RFID"
#define BUILD_TIMESTAMP __DATE__ " " __TIME__

// Note: Now using DFPlayer Mini for high-quality SD card audio playback
// Audio files must be on SD card as 0001.mp3, 0002.mp3, etc.

// Timing constants
const unsigned long COOLDOWN_PERIOD = 5000;        // 5 seconds cooldown between activations
const unsigned long STARTUP_LIGHT_DELAY = 500;     // Delay between startup light and sound
const unsigned long AUDIO_SETTLE_DELAY = 150;      // Delay after starting audio
const unsigned long MAIN_LOOP_DELAY = 100;         // Main loop iteration delay
const unsigned long DETECTION_WINDOW = 3000;       // 3 second detection window for RFID reading

// Cooldown management - prevent activations too close together
unsigned long last_activation = 0;

// Centralized band configurations with sound variations
// All band properties in one place: ID, name, color, sounds
BandConfig BAND_CONFIGS[] = {
  // Band 1 - Blue with Hello sound
  { 0x27CB1805, "August", CRGB::Blue, 
    { SOUND_PIRATE_CLIP }, 
    1, 0 },
  
  // Band 2 - Green with Foolish sound
  { 0xACD1E700, "Ophelia", CRGB::Green, 
    { SOUND_FOOLISH }, 
    1, 0 },
  
  // Band 3 - Purple with Operational sound
  { 0x34567890, "Evalette", CRGB::Purple, 
    { SOUND_ADDAMS_FAMILY }, 
    1, 0 },

  // Band 4 - Purple with Operational sound
  { 0x045C92F2876880ULL, "Candice", CRGB::Purple, 
    { SOUND_WIZARD_HARRY }, 
    1, 0 },

  // Band 5 - Purple with Operational sound
  { 0x56789012, "Danny", CRGB::Red, 
    { SOUND_VADER_BREATHING }, 
    1, 0 }
};

const int NUM_BANDS = sizeof(BAND_CONFIGS) / sizeof(BAND_CONFIGS[0]);

// Helper function to find band configuration by ID
// Returns pointer to BandConfig or nullptr if not found
BandConfig* find_band_config(uint64_t band_id) {
  for (int i = 0; i < NUM_BANDS; i++) {
    if (band_id == BAND_CONFIGS[i].band_id) {
      return &BAND_CONFIGS[i];
    }
  }
  return nullptr;
}

void setup() {

  // Initialize Serial for debugging (non-blocking)
  Serial.begin(115200);
  delay(100);  // Brief delay for Serial to stabilize

  DEBUG_PRINTLN("\n=== MagicBand (RFID) Initializing ===");
  DEBUG_PRINT("Firmware Version: ");
  DEBUG_PRINTLN(FIRMWARE_VERSION);
  DEBUG_PRINT("Build Timestamp: ");
  DEBUG_PRINTLN(BUILD_TIMESTAMP);
  DEBUG_PRINTLN("=========================================\n");
  DEBUG_PRINTLN("Comms enabled - beginning sensing");

  // CRITICAL INITIALIZATION ORDER!
  // I2C devices MUST be fully initialized before FastLED.show() is called
  // FastLED.show() disables interrupts which corrupts I2C bus state
  
  // STEP 1: Initialize RFID COMPLETELY (including firmware handshake)
  setup_rfid();
  
  // Check if RFID initialized successfully
  if (!is_rfid_initialized()) {
    DEBUG_PRINTLN("\n========================================");
    DEBUG_PRINTLN("⚠️  WARNING: RFID FAILED TO INITIALIZE");
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN("System will continue without RFID functionality.");
    DEBUG_PRINTLN("Check the error messages above for troubleshooting.");
    DEBUG_PRINTLN("You can still test LEDs, audio, and other features.");
    DEBUG_PRINTLN("========================================\n");
  } else {
    DEBUG_PRINTLN("[MAIN] ✓ RFID initialized successfully!\n");
  }
  
  // STEP 2: NOW safe to initialize LEDs and use FastLED.show()
  // I2C communication is complete, so interrupt disable won't affect it
  setup_leds();
  
  // Show immediate visual feedback - system is alive!
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.setBrightness(30);
  FastLED.show();
  
  // Audio Setup (DFPlayer Mini with SD card)
  // Non-blocking - if it fails, system continues without audio
  if (!setup_audio_dfplayer()) {
    DEBUG_PRINTLN("WARNING: DFPlayer Mini failed to initialize!");
    DEBUG_PRINTLN("Check wiring and SD card. System will continue without audio.");
  } else {
    DEBUG_PRINTLN("DFPlayer Mini ready - audio system online");
  }
  
  // Startup sequence - lights BEFORE WiFi to show system is alive
  DEBUG_PRINTLN("Starting magical startup sequence...");
  startup_light_sequence();
  
  // Home Assistant Setup (WiFi + MQTT) - done AFTER LED sequence
  // This way users see LEDs even if WiFi is slow/unavailable
  setup_home_assistant();
  
  // OTA Setup (must be after WiFi is connected)
  setup_ota();
  
  // Play startup sound if DFPlayer is ready (using chime as startup sound)
  if (dfplayer_is_ready()) {
    play_sound_file(SOUND_STARTOURS);
  }
  
  DEBUG_PRINTLN("MagicBand RFID system ready!");
  DEBUG_PRINT("Total startup time: ");
  DEBUG_PRINT(millis());
  DEBUG_PRINTLN("ms");
}

void loop() {
  
  // Handle OTA update requests (must be called frequently)
  loop_ota();
  
  // Process Home Assistant connection and commands
  loop_home_assistant();
  
  // Get current time for timing checks
  unsigned long current_time = millis();
  
  // Update stats for Home Assistant
  ha_stats.time_until_ready = (current_time - last_activation < get_ha_cooldown()) ? 
    (get_ha_cooldown() - (current_time - last_activation)) / 1000 : 0;
  
  // Check if system is enabled via Home Assistant
  if (!is_system_enabled()) {
    delay(MAIN_LOOP_DELAY);
    return; // Skip band detection if disabled
  }
  
  // Apply Home Assistant brightness setting
  uint8_t ha_brightness = get_ha_brightness();
  if (ha_brightness != FastLED.getBrightness()) {
    FastLED.setBrightness(ha_brightness);
  }
  
  // Use HA-controlled cooldown period
  unsigned long cooldown = get_ha_cooldown();
  
  // Check for RFID card detection (only when not in cooldown AND RFID is working)
  if (is_rfid_initialized() && is_rfid_card_present() && current_time - last_activation >= cooldown) {
    DEBUG_PRINTLN("RFID card detected! Starting read sequence...");
    
    // Play detection beep sound to indicate card detected
    if (dfplayer_is_ready()) {
      play_sound_file(SOUND_TAP_START);  // Quick beep to indicate detection started
      delay(300);
    }
    
    // Start the chase animation
    start_chase_animation();
    
    // Run the 3-second accelerating chase animation while trying to read the band
    unsigned long animation_start = millis();
    uint64_t band_id = 0;  // Use 64-bit to support both MIFARE and Magic Bands
    int read_attempts = 0;
    
    // Try to read during the animation window
    // IMPORTANT: Let animation run for full 3 seconds even if we read the band early
    while (millis() - animation_start < DETECTION_WINDOW) {
      update_chase_animation();
      
      // Only try to read if we haven't successfully read yet
      if (band_id == 0) {
        // Read and get the full 64-bit UID from current_band structure
        uint32_t temp_id = read_rfid_if_present();
        if (temp_id != 0) {
          // For 7+ byte UIDs, use the full 64-bit UID
          if (current_band.uid_length >= 7) {
            band_id = current_band.uid.uid_64;
          } else {
            // For 4-byte UIDs, use the 32-bit value
            band_id = temp_id;
          }
          DEBUG_PRINT("Successfully read band ID: 0x");
          if (current_band.uid_length >= 7) {
            DEBUG_PRINT((uint32_t)(band_id >> 32), HEX);
          }
          DEBUG_PRINTLN((uint32_t)(band_id & 0xFFFFFFFF), HEX);
          DEBUG_PRINT("Read attempts: ");
          DEBUG_PRINTLN(read_attempts + 1);
          // Don't break - let animation finish!
        }
        read_attempts++;
      }
      
      delay(10); // Small delay for animation smoothness
    }
    
    // Stop the chase animation
    stop_chase_animation();
    
    DEBUG_PRINT("Detection complete - Final ID: 0x");
    if (current_band.uid_length >= 7) {
      DEBUG_PRINT((uint32_t)(band_id >> 32), HEX);
    }
    DEBUG_PRINTLN((uint32_t)(band_id & 0xFFFFFFFF), HEX);
    
    // Check if we successfully read a band ID
    if (band_id != 0) {
      // Display UID in scanner format for easy identification
      DEBUG_PRINTLN("\n╔════════════════════════════════════════════════════╗");
      DEBUG_PRINTLN("║           RFID CARD DETECTED!                      ║");
      DEBUG_PRINTLN("╚════════════════════════════════════════════════════╝");
      
      // Get full band info with protocol and UID length
      uint8_t uid_length = current_band.uid_length;
      
      if (uid_length == 4) {
        // 4-byte UID (MIFARE)
        DEBUG_PRINTLN("Protocol: ISO 14443A (MIFARE)");
        DEBUG_PRINT("UID Length: ");
        DEBUG_PRINT(uid_length);
        DEBUG_PRINTLN(" bytes");
        DEBUG_PRINT("UID Bytes: ");
        for (uint8_t i = 0; i < uid_length; i++) {
          if (current_band.uid.uid_bytes[i] < 0x10) DEBUG_PRINT("0");
          DEBUG_PRINT(current_band.uid.uid_bytes[i], HEX);
          if (i < uid_length - 1) DEBUG_PRINT(" ");
        }
        DEBUG_PRINTLN();
        DEBUG_PRINT("UID as uint32_t: ");
        DEBUG_PRINT(band_id);
        DEBUG_PRINT(" (decimal) = 0x");
        DEBUG_PRINT(band_id, HEX);
        DEBUG_PRINTLN(" (hex)");
        DEBUG_PRINTLN();
        DEBUG_PRINT(">>> C++ Define: #define BAND_NAME 0x");
        DEBUG_PRINT(band_id, HEX);
        DEBUG_PRINTLN("UL");
      } else if (uid_length >= 7) {
        // 7+ byte UID (Magic Band / NFC Type 2)
        DEBUG_PRINTLN("Protocol: ISO 14443A (7+ byte UID - NFC Type 2)");
        DEBUG_PRINT("UID Length: ");
        DEBUG_PRINT(uid_length);
        DEBUG_PRINTLN(" bytes");
        DEBUG_PRINT("UID Bytes: ");
        for (uint8_t i = 0; i < uid_length; i++) {
          if (current_band.uid.uid_bytes[i] < 0x10) DEBUG_PRINT("0");
          DEBUG_PRINT(current_band.uid.uid_bytes[i], HEX);
          if (i < uid_length - 1) DEBUG_PRINT(" ");
        }
        DEBUG_PRINTLN();
        DEBUG_PRINT("UID as uint64_t: ");
        DEBUG_PRINT((uint32_t)(current_band.uid.uid_64 >> 32), HEX);
        DEBUG_PRINT((uint32_t)(current_band.uid.uid_64 & 0xFFFFFFFF), HEX);
        DEBUG_PRINTLN(" (hex)");
        DEBUG_PRINTLN();
        DEBUG_PRINT(">>> C++ Define (64-bit): #define BAND_NAME 0x");
        for (uint8_t i = 0; i < uid_length; i++) {
          if (current_band.uid.uid_bytes[i] < 0x10) DEBUG_PRINT("0");
          DEBUG_PRINT(current_band.uid.uid_bytes[i], HEX);
        }
        DEBUG_PRINTLN("ULL");
        DEBUG_PRINTLN();
        DEBUG_PRINT(">>> C++ Define (32-bit): #define BAND_NAME 0x");
        DEBUG_PRINT(band_id, HEX);
        DEBUG_PRINTLN("UL");
      }
      
      DEBUG_PRINTLN("────────────────────────────────────────────────────\n");
      
      // Search for matching band configuration
      BandConfig* band = find_band_config(band_id);
      
      if (band != nullptr) {
        DEBUG_PRINT("✓ Known RFID Band: ");
        DEBUG_PRINTLN(band->name);
        
        // Show band-specific color FIRST
        set_color(band->led_color);
        delay(200); // Brief moment to see the color
        
        // Play success chime while showing the color
        if (dfplayer_is_ready()) {
          play_sound_file(SOUND_CHIME);
          delay(1500); // Wait for chime to play completely
        }
        
        // Delay between chime and band sound (color stays on)
        delay(500);
        
        // Play current sound variation
        uint8_t sound_file = band->sound_files[band->current_sound_index];
        if (dfplayer_is_ready()) {
          play_sound_file(sound_file);
          delay(3000); // Let the sound play
        }
        
        // Fade out the color after a few seconds
        delay(1000);
        fade_out_leds();
        
        // Rotate to next sound for next activation
        band->current_sound_index = 
          (band->current_sound_index + 1) % band->num_sounds;
      } else {
        DEBUG_PRINTLN("⚠️  Unknown RFID Band - Not in configuration");
        DEBUG_PRINTLN("   Copy the define above and add to BandConfig.h");
        
        // Flash red and play error sound
        flash_color(CRGB::Red, 3, 200);
        if (dfplayer_is_ready()) {
          play_sound_file(SOUND_ERROR);
          delay(1500);
        }
      }
      
      // Publish band activation to Home Assistant
      publish_wand_activation(band_id);

    } else {
      DEBUG_PRINTLN("Failed to read band UID during 3-second window");
      
      // Flash red and play error sound
      flash_color(CRGB::Red, 3, 200);
      if (dfplayer_is_ready()) {
        play_sound_file(SOUND_ERROR);
        delay(1500);
      }
    }
    
    last_activation = current_time;
    
  } else if (current_time - last_activation < cooldown && last_activation > 0) {
    // Show cooldown visual feedback
    cooldown_pulse();
  }

  // wait a bit, and then back to receiving and decoding
  delay(MAIN_LOOP_DELAY);
}
