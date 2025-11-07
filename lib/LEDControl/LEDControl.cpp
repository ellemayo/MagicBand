

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
  DEBUG_PRINTLN("Starting chase animation");
  
  // Save original brightness
  uint8_t original_brightness = FastLED.getBrightness();
  FastLED.setBrightness(LED_DEFAULT_BRIGHTNESS);
  
  for (int cycle = 0; cycle < num_cycles; cycle++) {
    // Forward chase
    for (int i = 0; i < NUM_LEDS; i++) {
      // Turn off all LEDs
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      
      // Light up current LED and a trailing tail
      leds[i] = color;
      if (i > 0) leds[i-1] = color; // Tail 1
      leds[i-1].fadeToBlackBy(128);
      if (i > 1) leds[i-2] = color; // Tail 2
      leds[i-2].fadeToBlackBy(192);
      
      FastLED.show();
      delay(speed_ms);
    }
    
    // Optional: Reverse chase
    for (int i = NUM_LEDS - 1; i >= 0; i--) {
      // Turn off all LEDs
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      
      // Light up current LED and a trailing tail
      leds[i] = color;
      if (i < NUM_LEDS - 1) leds[i+1] = color; // Tail 1
      leds[i+1].fadeToBlackBy(128);
      if (i < NUM_LEDS - 2) leds[i+2] = color; // Tail 2
      leds[i+2].fadeToBlackBy(192);
      
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
