#include <Arduino.h>
#include <DebugConfig.h>

#include <IRControl.h>
#include <LEDControl.h>
#include <ServoControl.h>
#include <AudioControlDFPlayer.h>
#include <HomeAssistantControl.h>
#include <OTAControl.h>

// Firmware version for tracking OTA updates
#define FIRMWARE_VERSION "1.0.1-OTA"
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

// Wand configuration structure with sound variation support
struct WandConfig {
  uint32_t wand_id;
  CRGB led_color;
  uint8_t sound_files[3];        // Array of up to 3 sound file numbers
  uint8_t num_sounds;            // Number of sounds available for this wand
  uint8_t current_sound_index;   // Current position in sound rotation
};

// Centralized wand configurations with sound variations
WandConfig WAND_CONFIGS[] = {
  // Wand 1 (Blue - August) - Mystical and magical sounds
  { WAND_1, CRGB::Blue, 
    { SOUND_CHIMES, SOUND_MAGIC_SPELL, SOUND_WIND_MAGIC }, 
    3, 0 },
  
  // Wand 2 (Green - Ophelia) - Nature and healing sounds
  { WAND_2, CRGB::Green, 
    { SOUND_SPARKLE, SOUND_HEALING_MAGIC, SOUND_HEALING_MAGIC_2 }, 
    3, 0 },
  
  // Wand 3 (Purple - Evalette) - Powerful spell sounds
  { WAND_3, CRGB::Purple, 
    { SOUND_FIRE_MAGIC, SOUND_MAGIC_SPELL_2, SOUND_MAGIC_WAND }, 
    3, 0 }
};

const int NUM_WANDS = sizeof(WAND_CONFIGS) / sizeof(WAND_CONFIGS[0]);

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

  DEBUG_PRINTLN("\n=== MagiQuest Box Initializing ===");
  DEBUG_PRINT("Firmware Version: ");
  DEBUG_PRINTLN(FIRMWARE_VERSION);
  DEBUG_PRINT("Build Timestamp: ");
  DEBUG_PRINTLN(BUILD_TIMESTAMP);
  DEBUG_PRINTLN("==================================\n");
  DEBUG_PRINTLN("Comms enabled - beginning sensing");

  // Connect IR receiver
  setup_ir();
  
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
  
  DEBUG_PRINTLN("MagiQuest system ready!");
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
    return; // Skip wand detection if disabled
  }
  
  // Apply Home Assistant brightness setting
  uint8_t ha_brightness = get_ha_brightness();
  if (ha_brightness != FastLED.getBrightness()) {
    FastLED.setBrightness(ha_brightness);
  }
  
  // Check for IR input
  uint32_t wand_id = loop_ir();
  
  // Get current time
  unsigned long current_time = millis();
  
  // Use HA-controlled cooldown period
  unsigned long cooldown = get_ha_cooldown();
  
  // Check if enough time has passed since last activation (applies to all wands)
  if (wand_id != 0 && current_time - last_activation >= cooldown) {
    // Search for matching wand configuration
    bool wand_found = false;
    for (int i = 0; i < NUM_WANDS; i++) {
      if (wand_id == WAND_CONFIGS[i].wand_id) {
        DEBUG_PRINT("Wand activated (ID: ");
        DEBUG_PRINT(wand_id);
        DEBUG_PRINT(") - Sound variation: ");
        DEBUG_PRINTLN(WAND_CONFIGS[i].current_sound_index + 1);
        
        set_color(WAND_CONFIGS[i].led_color);
        // Removed POWER_SETTLE_DELAY - not needed with proper power supply
        
        // Play current sound variation
        uint8_t sound_file = WAND_CONFIGS[i].sound_files[WAND_CONFIGS[i].current_sound_index];
        if (dfplayer_is_ready()) {
          play_sound_file(sound_file);
        }
        
        // Reduced audio settle delay - DFPlayer buffers internally
        delay(150); // Reduced from 300ms
        
        // Rotate to next sound for next activation
        WAND_CONFIGS[i].current_sound_index = 
          (WAND_CONFIGS[i].current_sound_index + 1) % WAND_CONFIGS[i].num_sounds;
        
        toggle_lid();
        
        // Publish wand activation to Home Assistant
        publish_wand_activation(wand_id);
        
        wand_found = true;
        break;
      }
    }
    
    if (!wand_found) {
      DEBUG_PRINT("Unknown wand ID: ");
      DEBUG_PRINTLN(wand_id);
    }
    
    last_activation = current_time; // Update last activation time for any wand
  } else if (wand_id != 0) {
    DEBUG_PRINTLN("Wand on cooldown - ignoring");
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
