#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <FastLED.h>

// LED strip configuration
#define NUM_LEDS 17
#define DATA_PIN 13

// LED brightness and timing constants
#define LED_DEFAULT_BRIGHTNESS 80
#define LED_MAX_VOLTAGE 5
#define LED_MAX_MILLIAMPS 200
#define LED_FADE_STEP_DELAY 20  // Delay per brightness step during fade (ms)

// Define the array of leds
extern CRGB leds[NUM_LEDS];

void setup_leds();
bool set_color(CRGB color);
void startup_light_sequence();
void turn_off_leds(); // Turn off all LEDs
void fade_out_leds(); // Fade out LEDs gradually
void cooldown_pulse(); // Subtle pulse effect during cooldown
void chase_animation(CRGB color, int speed_ms = 50, int num_cycles = 3); // Chase animation effect
void accelerating_chase(CRGB color); // Chase that starts slow and speeds up
void fade_in_out(CRGB color, int fade_speed_ms = 20); // Fade in to color, then fade out
void flash_color(CRGB color, int num_flashes = 3, int flash_speed_ms = 200); // Flash a color multiple times

// Non-blocking chase animation state management
void start_chase_animation(); // Start the chase animation (call once when RFID detected)
bool update_chase_animation(); // Update chase animation (call every loop iteration), returns true when animation is done
void stop_chase_animation(); // Stop the chase animation immediately

#endif