#include <Arduino.h>
#include <DebugConfig.h>

#include <RFIDControl.h>
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

// Band configuration structure with sound variation support
struct BandConfig {
  uint32_t band_id;
  CRGB led_color;
  uint8_t sound_files[3];        // Array of up to 3 sound file numbers
  uint8_t num_sounds;            // Number of sounds available for this band
  uint8_t current_sound_index;   // Current position in sound rotation
};

// Centralized band configurations with sound variations
BandConfig BAND_CONFIGS[] = {
  // Band 1 (Blue) - Hello sound
  { BAND_1, CRGB::Blue, 
    { SOUND_HELLO }, 
    1, 0 },
  
  // Band 2 (Green) - Star Tours sound
  { BAND_2, CRGB::Green, 
    { SOUND_FOOLISH }, 
    1, 0 },
  
  // Band 3 (Purple) - Operational sound
  { BAND_3, CRGB::Purple, 
    { SOUND_OPERATIONAL }, 
    1, 0 },

  // Band 4 (Purple) - Operational sound
  { BAND_4, CRGB::Purple, 
    { SOUND_OPERATIONAL }, 
    1, 0 },

  // Band 5 (Purple) - Operational sound
  { BAND_5, CRGB::Purple, 
    { SOUND_OPERATIONAL }, 
    1, 0 }
};

const int NUM_BANDS = sizeof(BAND_CONFIGS) / sizeof(BAND_CONFIGS[0]);

void setup() {

  // Initialize Serial for debugging (non-blocking)
  Serial.begin(115200);
  
  // CRITICAL: Initialize LEDs FIRST before anything else
  // This provides immediate visual feedback that the device is booting
  // Even if other components fail, users will see the LEDs working
  setup_leds();
  
  // Show immediate visual feedback - system is alive!
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.setBrightness(30);
  FastLED.show();
  
  // Brief delay for Serial to stabilize (non-blocking, no wait)
  delay(100);

  DEBUG_PRINTLN("\n=== MagicBand (RFID) Initializing ===");
  DEBUG_PRINT("Firmware Version: ");
  DEBUG_PRINTLN(FIRMWARE_VERSION);
  DEBUG_PRINT("Build Timestamp: ");
  DEBUG_PRINTLN(BUILD_TIMESTAMP);
  DEBUG_PRINTLN("=========================================\n");
  DEBUG_PRINTLN("Comms enabled - beginning sensing");

  // CRITICAL: Initialize RFID AFTER LEDs because FastLED can interfere with SPI
  // FastLED.show() disables interrupts briefly which can disrupt SPI initialization
  setup_rfid();
  
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
    play_sound_file(SOUND_CHIME);
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
  
  // Check for RFID card detection (only when not in cooldown)
  if (is_rfid_card_present() && current_time - last_activation >= cooldown) {
    DEBUG_PRINTLN("RFID card detected! Attempting immediate read...");
    
    // Try to read the card IMMEDIATELY before starting animation
    // Give it a moment to stabilize and try a few times
    uint32_t band_id = 0;
    for (int initial_attempt = 0; initial_attempt < 3 && band_id == 0; initial_attempt++) {
      if (initial_attempt > 0) {
        delay(30); // Small delay between attempts
      }
      band_id = read_rfid_if_present();
    }
    
    if (band_id != 0) {
      DEBUG_PRINT("Successfully read band ID on initial detection: 0x");
      DEBUG_PRINTLN(band_id, HEX);
    } else {
      DEBUG_PRINTLN("Initial read failed after 3 attempts, will retry during animation");
    }
    
    // Play detection beep sound to indicate card detected
    if (dfplayer_is_ready()) {
      play_sound_file(SOUND_CHIME);  // Quick beep to indicate detection started
      delay(300);
    }
    
    // Start the chase animation
    start_chase_animation();
    
    // Run the 3-second accelerating chase animation (blocking)
    unsigned long animation_start = millis();
    int read_attempts = 0;
    int successful_reads = 0;
    
    // If we already have the ID, count it as first successful read
    if (band_id != 0) {
      read_attempts = 1;
      successful_reads = 1;
    }
    
    while (millis() - animation_start < DETECTION_WINDOW) {
      update_chase_animation();
      
      // Only try to read if we don't have an ID yet
      if (band_id == 0 && millis() - animation_start > 100) { // Wait 100ms before retry attempts
        uint32_t temp_id = read_rfid_if_present();
        read_attempts++;
        
        if (temp_id != 0) {
          successful_reads++;
          DEBUG_PRINT("Read attempt ");
          DEBUG_PRINT(read_attempts);
          DEBUG_PRINT(" - Got ID: 0x");
          DEBUG_PRINTLN(temp_id, HEX);
          
          // Use this ID since we don't have one yet
          band_id = temp_id;
          DEBUG_PRINTLN("ID locked in from retry attempt");
        }
      }
      
      delay(10); // Small delay for animation smoothness
    }
    
    // Stop the chase animation
    stop_chase_animation();
    
    DEBUG_PRINT("Detection complete - Read attempts: ");
    DEBUG_PRINT(read_attempts);
    DEBUG_PRINT(" | Successful reads: ");
    DEBUG_PRINT(successful_reads);
    DEBUG_PRINT(" | Final ID: 0x");
    DEBUG_PRINTLN(band_id, HEX);
    
    // Check if we successfully read a band ID
    if (band_id != 0) {
      // Search for matching band configuration
      bool band_found = false;
      for (int i = 0; i < NUM_BANDS; i++) {
        if (band_id == BAND_CONFIGS[i].band_id) {
          DEBUG_PRINT("Known RFID Band activated (ID: 0x");
          DEBUG_PRINT(band_id, HEX);
          DEBUG_PRINTLN(")");
          
          // Show band-specific color FIRST
          set_color(BAND_CONFIGS[i].led_color);
          delay(200); // Brief moment to see the color
          
          // Play success chime while showing the color
          if (dfplayer_is_ready()) {
            play_sound_file(SOUND_CHIME);
            delay(1500); // Wait for chime to play completely
          }
          
          // Delay between chime and band sound (color stays on)
          delay(500);
          
          // Play current sound variation
          uint8_t sound_file = BAND_CONFIGS[i].sound_files[BAND_CONFIGS[i].current_sound_index];
          if (dfplayer_is_ready()) {
            play_sound_file(sound_file);
            delay(3000); // Let the sound play
          }
          
          // Fade out the color after a few seconds
          delay(1000);
          fade_out_leds();
          
          // Rotate to next sound for next activation
          BAND_CONFIGS[i].current_sound_index = 
            (BAND_CONFIGS[i].current_sound_index + 1) % BAND_CONFIGS[i].num_sounds;
          
          // Publish band activation to Home Assistant
          publish_wand_activation(band_id);
          
          band_found = true;
          break;
        }
      }
      
      if (!band_found) {
        DEBUG_PRINT("Unknown RFID Band ID: 0x");
        DEBUG_PRINTLN(band_id, HEX);
        
        // Flash red and play error sound
        flash_color(CRGB::Red, 3, 200);
        if (dfplayer_is_ready()) {
          play_sound_file(SOUND_ERROR);
          delay(1500);
        }
      }
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
