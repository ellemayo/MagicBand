#include <Arduino.h>
#include <DebugConfig.h>

#include <BandConfig.h>
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

// Centralized band configurations with sound variations
// All band properties in one place: ID, name, color, sounds
BandConfig BAND_CONFIGS[] = {
  // Band 1 - Blue with Hello sound
  { 0x27CB1805, "Band 1", CRGB::Blue, 
    { SOUND_HELLO }, 
    1, 0 },
  
  // Band 2 - Green with Foolish sound
  { 0xACD1E700, "Band 2", CRGB::Green, 
    { SOUND_FOOLISH }, 
    1, 0 },
  
  // Band 3 - Purple with Operational sound
  { 0x34567890, "Band 3", CRGB::Purple, 
    { SOUND_OPERATIONAL }, 
    1, 0 },

  // Band 4 - Purple with Operational sound
  { 0x45678901, "Band 4", CRGB::Purple, 
    { SOUND_OPERATIONAL }, 
    1, 0 },

  // Band 5 - Purple with Operational sound
  { 0x56789012, "Band 5", CRGB::Purple, 
    { SOUND_OPERATIONAL }, 
    1, 0 }
};

const int NUM_BANDS = sizeof(BAND_CONFIGS) / sizeof(BAND_CONFIGS[0]);

// Helper function to find band configuration by ID
// Returns pointer to BandConfig or nullptr if not found
BandConfig* find_band_config(uint32_t band_id) {
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
    DEBUG_PRINTLN("RFID card detected! Starting read sequence...");
    
    // Play detection beep sound to indicate card detected
    if (dfplayer_is_ready()) {
      play_sound_file(SOUND_CHIME);  // Quick beep to indicate detection started
      delay(300);
    }
    
    // Start the chase animation
    start_chase_animation();
    
    // Run the 3-second accelerating chase animation while trying to read the band
    unsigned long animation_start = millis();
    uint32_t band_id = 0;
    int read_attempts = 0;
    
    // Try to read during the animation window
    // IMPORTANT: Let animation run for full 3 seconds even if we read the band early
    while (millis() - animation_start < DETECTION_WINDOW) {
      update_chase_animation();
      
      // Only try to read if we haven't successfully read yet
      if (band_id == 0) {
        uint32_t temp_id = read_rfid_if_present();
        if (temp_id != 0) {
          band_id = temp_id;
          DEBUG_PRINT("Successfully read band ID: 0x");
          DEBUG_PRINTLN(band_id, HEX);
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
    DEBUG_PRINTLN(band_id, HEX);
    
    // Check if we successfully read a band ID
    if (band_id != 0) {
      // Search for matching band configuration
      BandConfig* band = find_band_config(band_id);
      
      if (band != nullptr) {
        DEBUG_PRINT("Known RFID Band activated: ");
        DEBUG_PRINT(band->name);
        DEBUG_PRINT(" (ID: 0x");
        DEBUG_PRINT(band_id, HEX);
        DEBUG_PRINTLN(")");
        
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
        
        // Publish band activation to Home Assistant
        publish_wand_activation(band_id);
      } else {
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
