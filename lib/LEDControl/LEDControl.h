#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <FastLED.h>

// LED strip configuration
#define NUM_LEDS 24
#define DATA_PIN 3

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

#endif