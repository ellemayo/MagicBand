

#include <LEDControl.h>
#include <DebugConfig.h>

CRGB leds[NUM_LEDS];

void setup_leds() {
  // Register LEDS with delay to ensure stable power
  // Small delay helps when switching from USB to DC power
  delay(50);
  
  //FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_MAX_VOLTAGE, LED_MAX_MILLIAMPS);
  FastLED.setBrightness(LED_DEFAULT_BRIGHTNESS);

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  delay(10);  // Brief delay to ensure first show() completes
}

bool set_color(CRGB color) {
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();

  return true;
}

// Startup light sequence - magical power-on animation
// Optimized to reduce blocking time
void startup_light_sequence() {
  DEBUG_PRINTLN("Starting LED startup sequence");
  
  // Turn off all LEDs first
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  delay(100); // Reduced from 200ms
  
  // Rainbow sweep effect
  for (int brightness = 0; brightness <= 120; brightness += 10) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(i * 255 / NUM_LEDS, 255, brightness);
    }
    FastLED.show();
    delay(30); // Reduced from 50ms
  }
  
  // Fade to white and then to black - faster
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.setBrightness(60);
  FastLED.show();
  delay(200); // Reduced from 500ms
  
  // Fade out
  for (int brightness = 60; brightness >= 0; brightness -= 5) {
    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(50);
  }
  
  // Set back to normal brightness and turn off
  FastLED.setBrightness(LED_DEFAULT_BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  DEBUG_PRINTLN("LED startup sequence complete");
}

// Turn off all LEDs
void turn_off_leds() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

// Fade out LEDs gradually
void fade_out_leds() {
  int current_brightness = FastLED.getBrightness();
  
  // Fade from current brightness down to 0
  for (int brightness = current_brightness; brightness >= 0; brightness -= 1) {
    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(LED_FADE_STEP_DELAY);
  }
  
  // Turn off LEDs and restore original brightness
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.setBrightness(LED_DEFAULT_BRIGHTNESS);
  FastLED.show();
}

// Subtle pulse effect for cooldown - non-blocking
// Pulses the LEDs at their current color (from last wand activation)
void cooldown_pulse() {
  static unsigned long last_pulse = 0;
  static uint8_t pulse_brightness = 0;
  static bool pulse_direction = true;  // true = getting brighter, false = getting dimmer
  
  unsigned long current_time = millis();
  
  // Update pulse every 30ms for smooth animation
  if (current_time - last_pulse >= 30) {
    last_pulse = current_time;
    
    // Adjust brightness - pulse from 10 to 80
    if (pulse_direction) {
      pulse_brightness += 5;
      if (pulse_brightness >= 80) {
        pulse_direction = false;
      }
    } else {
      if (pulse_brightness <= 10) {
        pulse_brightness = 10;
        pulse_direction = true;
      } else {
        pulse_brightness -= 5;
      }
    }
    
    // Keep LEDs at their current color, just adjust brightness
    // This preserves the last wand's color during cooldown
    FastLED.setBrightness(pulse_brightness);
    FastLED.show();
  }
}

// Chase animation - light moves along the strip
// color: The color to use for the chase effect
// speed_ms: Delay between each step (lower = faster)
// num_cycles: How many times to run the full chase
void chase_animation(CRGB color, int speed_ms, int num_cycles) {
  DEBUG_PRINTLN("Starting chase animation (skipping first LED)");
  
  // Save original brightness
  uint8_t original_brightness = FastLED.getBrightness();
  FastLED.setBrightness(LED_DEFAULT_BRIGHTNESS);
  
  for (int cycle = 0; cycle < num_cycles; cycle++) {
    // Forward chase - start at LED 1 (skip LED 0)
    for (int i = 1; i < NUM_LEDS; i++) {
      // Turn off all LEDs except first one
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      
      // Light up current LED and a trailing tail
      leds[i] = color;
      if (i > 1) {  // Changed from i > 0 to avoid lighting LED 0
        leds[i-1] = color; // Tail 1
        leds[i-1].fadeToBlackBy(128);
      }
      if (i > 2) {  // Changed from i > 1
        leds[i-2] = color; // Tail 2
        leds[i-2].fadeToBlackBy(192);
      }
      
      FastLED.show();
      delay(speed_ms);
    }
    
    // Optional: Reverse chase - end at LED 1 (skip LED 0)
    for (int i = NUM_LEDS - 1; i >= 1; i--) {  // Changed from i >= 0
      // Turn off all LEDs except first one
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      
      // Light up current LED and a trailing tail
      leds[i] = color;
      if (i < NUM_LEDS - 1) {
        leds[i+1] = color; // Tail 1
        leds[i+1].fadeToBlackBy(128);
      }
      if (i < NUM_LEDS - 2) {
        leds[i+2] = color; // Tail 2
        leds[i+2].fadeToBlackBy(192);
      }
      
      FastLED.show();
      delay(speed_ms);
    }
  }
  
  // Clear strip
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  // Restore original brightness
  FastLED.setBrightness(original_brightness);
  
  DEBUG_PRINTLN("Chase animation complete");
}

// Accelerating chase - starts slow and speeds up
// Creates excitement as detection happens
void accelerating_chase(CRGB color) {
  DEBUG_PRINTLN("Starting accelerating chase animation (skipping first LED)");
  
  // Save original brightness
  uint8_t original_brightness = FastLED.getBrightness();
  FastLED.setBrightness(LED_DEFAULT_BRIGHTNESS);
  
  // Start slow, then speed up progressively
  int speeds[] = {150, 120, 90, 60, 40, 25, 15, 10};  // Decreasing delays = faster
  int num_speeds = sizeof(speeds) / sizeof(speeds[0]);
  
  for (int speed_idx = 0; speed_idx < num_speeds; speed_idx++) {
    int speed_ms = speeds[speed_idx];
    
    // Do one pass at this speed - start at LED 1 (skip LED 0)
    for (int i = 1; i < NUM_LEDS; i++) {
      // Turn off all LEDs except first one
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      
      // Light up current LED and a trailing tail
      leds[i] = color;
      if (i > 1) {  // Changed from i > 0 to avoid lighting LED 0
        leds[i-1] = color;
        leds[i-1].fadeToBlackBy(128);
      }
      if (i > 2) {  // Changed from i > 1
        leds[i-2] = color;
        leds[i-2].fadeToBlackBy(192);
      }
      
      FastLED.show();
      delay(speed_ms);
    }
  }
  
  // Final flash at the end - but keep first LED off
  fill_solid(leds, NUM_LEDS, color);
  leds[0] = CRGB::Black;  // Keep first LED off
  FastLED.show();
  delay(100);
  
  // Restore original brightness
  FastLED.setBrightness(original_brightness);
  
  DEBUG_PRINTLN("Accelerating chase complete");
}

// Fade in to a color, hold, then fade out
// Perfect for success indication
void fade_in_out(CRGB color, int fade_speed_ms) {
  DEBUG_PRINTLN("Starting fade in/out animation");
  
  // Save original brightness
  uint8_t original_brightness = FastLED.getBrightness();
  
  // Fill with color but at zero brightness
  fill_solid(leds, NUM_LEDS, color);
  FastLED.setBrightness(0);
  FastLED.show();
  
  // Fade in
  for (int brightness = 0; brightness <= LED_DEFAULT_BRIGHTNESS; brightness += 5) {
    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(fade_speed_ms);
  }
  
  // Hold at full brightness
  delay(500);
  
  // Fade out
  for (int brightness = LED_DEFAULT_BRIGHTNESS; brightness >= 0; brightness -= 5) {
    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(fade_speed_ms);
  }
  
  // Clear and restore
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.setBrightness(original_brightness);
  FastLED.show();
  
  DEBUG_PRINTLN("Fade in/out complete");
}

// Flash a color multiple times
// Perfect for error/fail indication
void flash_color(CRGB color, int num_flashes, int flash_speed_ms) {
  DEBUG_PRINTLN("Starting flash animation");
  
  // Save original brightness
  uint8_t original_brightness = FastLED.getBrightness();
  FastLED.setBrightness(LED_DEFAULT_BRIGHTNESS);
  
  for (int i = 0; i < num_flashes; i++) {
    // Flash on
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
    delay(flash_speed_ms);
    
    // Flash off
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(flash_speed_ms);
  }
  
  // Restore original brightness
  FastLED.setBrightness(original_brightness);
  
  DEBUG_PRINTLN("Flash animation complete");
}

// Non-blocking chase animation state variables
static bool chase_active = false;
static unsigned long chase_start_time = 0;
static unsigned long chase_last_update = 0;
static int chase_position = 0;
static int chase_speed_ms = 150; // Current speed
static uint8_t saved_brightness = LED_DEFAULT_BRIGHTNESS; // Save brightness for restore

// Start the chase animation (call when RFID is first detected)
void start_chase_animation() {
  DEBUG_PRINTLN("Starting non-blocking chase animation");
  chase_active = true;
  chase_start_time = millis();
  chase_last_update = millis();
  chase_position = 1;  // Start at LED 1 instead of 0
  chase_speed_ms = 150; // Start slow
  
  // Save current brightness and set to default
  saved_brightness = FastLED.getBrightness();
  FastLED.setBrightness(LED_DEFAULT_BRIGHTNESS);
  
  // Clear LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

// Update chase animation (call every loop iteration)
// Returns true when the 3-second animation is complete
bool update_chase_animation() {
  if (!chase_active) {
    return true; // Not running, so it's "done"
  }
  
  unsigned long current_time = millis();
  unsigned long elapsed_time = current_time - chase_start_time;
  
  // Animation runs for 3 seconds
  const unsigned long ANIMATION_DURATION = 3000;
  
  // Check if animation is complete
  if (elapsed_time >= ANIMATION_DURATION) {
    DEBUG_PRINTLN("Chase animation complete (3 seconds elapsed)");
    chase_active = false;
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    return true;
  }
  
  // Calculate speed based on elapsed time (accelerate over 3 seconds)
  // Start at 150ms, end at 10ms
  float progress = (float)elapsed_time / ANIMATION_DURATION;
  chase_speed_ms = 150 - (int)(140 * progress); // 150 -> 10
  if (chase_speed_ms < 10) chase_speed_ms = 10;
  
  // Update position if enough time has passed
  if (current_time - chase_last_update >= chase_speed_ms) {
    chase_last_update = current_time;
    
    // Turn off all LEDs except first one
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    
    // Skip LED 0 - if position is 0, move to 1
    if (chase_position == 0) {
      chase_position = 1;
    }
    
    // Light up current LED and trailing tail with cyan/blue color (more visible)
    CRGB chase_color = CRGB(0, 150, 255); // Bright cyan-blue
    leds[chase_position] = chase_color;
    
    // Add trailing tail - but never light up LED 0
    if (chase_position > 1) {  // Changed from chase_position > 0
      leds[chase_position - 1] = chase_color;
      leds[chase_position - 1].fadeToBlackBy(128);
    }
    
    if (chase_position > 2) {  // Changed from chase_position > 1
      leds[chase_position - 2] = chase_color;
      leds[chase_position - 2].fadeToBlackBy(192);
    }
    
    FastLED.show();
    
    // Move to next position
    chase_position++;
    if (chase_position >= NUM_LEDS) {
      chase_position = 1; // Loop back to LED 1 (skip LED 0)
    }
  }
  
  return false; // Animation still running
}

// Stop the chase animation immediately
void stop_chase_animation() {
  DEBUG_PRINTLN("Stopping chase animation");
  chase_active = false;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  // Restore original brightness
  FastLED.setBrightness(saved_brightness);
}
