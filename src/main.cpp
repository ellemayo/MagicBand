#include <Arduino.h>
#include <DebugConfig.h>

#include <RFIDControl.h>  // Changed from IRControl
#include <LEDControl.h>
#include <ServoControl.h>
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
const unsigned long POWER_SETTLE_DELAY = 50;       // Delay after LEDs before servo to prevent brownout
const unsigned long AUDIO_SETTLE_DELAY = 300;      // Delay after starting audio before servo (prevents audio glitches)
const unsigned long MAIN_LOOP_DELAY = 100;         // Main loop iteration delay

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
  // Band 1 (Blue - August) - Mystical and magical sounds
  { BAND_1, CRGB::Blue, 
    { SOUND_CHIMES, SOUND_MAGIC_SPELL, SOUND_WIND_MAGIC }, 
    3, 0 },
  
  // Band 2 (Green - Ophelia) - Nature and healing sounds
  { BAND_2, CRGB::Green, 
    { SOUND_SPARKLE, SOUND_HEALING_MAGIC, SOUND_HEALING_MAGIC_2 }, 
    3, 0 },
  
  // Band 3 (Purple - Evalette) - Powerful spell sounds
  { BAND_3, CRGB::Purple, 
    { SOUND_FIRE_MAGIC, SOUND_MAGIC_SPELL_2, SOUND_MAGIC_WAND }, 
    3, 0 }
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

  DEBUG_PRINTLN("\n=== MagiQuest Box (RFID) Initializing ===");
  DEBUG_PRINT("Firmware Version: ");
  DEBUG_PRINTLN(FIRMWARE_VERSION);
  DEBUG_PRINT("Build Timestamp: ");
  DEBUG_PRINTLN(BUILD_TIMESTAMP);
  DEBUG_PRINTLN("=========================================\n");
  DEBUG_PRINTLN("Comms enabled - beginning sensing");

  // Connect RFID reader (replaces IR receiver)
  setup_rfid();
  
  // Connect Servo
  setup_servo();
  
  // Audio Setup (DFPlayer Mini with SD card)
  // Non-blocking - if it fails, system continues without audio
  if (!setup_audio_dfplayer()) {
    DEBUG_PRINTLN("WARNING: DFPlayer Mini failed to initialize!");
    DEBUG_PRINTLN("Check wiring and SD card. System will continue without audio.");
  } else {
    DEBUG_PRINTLN("DFPlayer Mini ready - audio system online");
    set_volume(30);  // Set comfortable default volume
  }
  
  // Startup sequence - lights BEFORE WiFi to show system is alive
  DEBUG_PRINTLN("Starting magical startup sequence...");
  startup_light_sequence();
  
  // Home Assistant Setup (WiFi + MQTT) - done AFTER LED sequence
  // This way users see LEDs even if WiFi is slow/unavailable
  setup_home_assistant();
  
  // OTA Setup (must be after WiFi is connected)
  setup_ota();
  
  // Play startup sound if DFPlayer is ready (using chimes as startup sound)
  if (dfplayer_is_ready()) {
    play_sound_file(SOUND_CHIMES);
  }
  
  DEBUG_PRINTLN("MagiQuest RFID system ready!");
  DEBUG_PRINT("Total startup time: ");
  DEBUG_PRINT(millis());
  DEBUG_PRINTLN("ms");
}

void loop() {
  
  // Handle OTA update requests (must be called frequently)
  loop_ota();
  
  // Process Home Assistant connection and commands
  loop_home_assistant();
  
  // Update stats for Home Assistant
  ha_stats.lid_is_open = lid_is_open;
  ha_stats.time_until_ready = (millis() - last_activation < get_ha_cooldown()) ? 
    (get_ha_cooldown() - (millis() - last_activation)) / 1000 : 0;
  
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
  
  // Check for RFID input (replaces IR wand detection)
  uint32_t band_id = loop_rfid();
  
  // Get current time
  unsigned long current_time = millis();
  
  // Use HA-controlled cooldown period
  unsigned long cooldown = get_ha_cooldown();
  
  // Check if enough time has passed since last activation (applies to all bands)
  if (band_id != 0 && current_time - last_activation >= cooldown) {
    // Search for matching band configuration
    bool band_found = false;
    for (int i = 0; i < NUM_BANDS; i++) {
      if (band_id == BAND_CONFIGS[i].band_id) {
        DEBUG_PRINT("RFID Band activated (ID: 0x");
        DEBUG_PRINT(band_id, HEX);
        DEBUG_PRINT(") - Sound variation: ");
        DEBUG_PRINTLN(BAND_CONFIGS[i].current_sound_index + 1);
        
        set_color(BAND_CONFIGS[i].led_color);
        // Removed POWER_SETTLE_DELAY - not needed with proper power supply
        
        // Play current sound variation
        uint8_t sound_file = BAND_CONFIGS[i].sound_files[BAND_CONFIGS[i].current_sound_index];
        if (dfplayer_is_ready()) {
          play_sound_file(sound_file);
        }
        
        // Reduced audio settle delay - DFPlayer buffers internally
        delay(150); // Reduced from 300ms
        
        // Rotate to next sound for next activation
        BAND_CONFIGS[i].current_sound_index = 
          (BAND_CONFIGS[i].current_sound_index + 1) % BAND_CONFIGS[i].num_sounds;
        
        toggle_lid();
        
        // Publish band activation to Home Assistant
        publish_wand_activation(band_id);  // Reuses existing HA function
        
        band_found = true;
        break;
      }
    }
    
    if (!band_found) {
      DEBUG_PRINT("Unknown RFID Band ID: 0x");
      DEBUG_PRINTLN(band_id, HEX);
    }
    
    last_activation = current_time; // Update last activation time for any band
  } else if (band_id != 0) {
    DEBUG_PRINTLN("Band on cooldown - ignoring");
  }
  
  // Show cooldown visual feedback if in cooldown period
  if (current_time - last_activation < cooldown && last_activation > 0) {
    cooldown_pulse();
  }
  
  // Check if lid should auto-close (if enabled via HA)
  if (is_auto_close_enabled()) {
    check_auto_close();
  }
  
  // wait a bit, and then back to receiving and decoding
  delay(MAIN_LOOP_DELAY);
}
