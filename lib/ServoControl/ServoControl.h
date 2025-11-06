#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <ESP32Servo.h>

// Pin definitions
#define SERVO_PIN 18

// Timing constants
#define AUTO_CLOSE_TIMEOUT 15000  // Auto-close timeout in milliseconds (15 seconds)
#define SERVO_STEP_DELAY 15       // Delay per servo degree movement (ms)
#define SERVO_CLOSE_PAUSE 100     // Pause after lid closes before LED fade (ms)

extern Servo myservo;  // create servo object to control a servo
extern int servo_pos;    // variable to store the servo position
extern bool lid_is_open; // track if lid is currently open
extern unsigned long last_lid_open_time; // track when lid was last opened

void setup_servo();
void open_lid();  // Open the lid (move to position 0)
void close_lid(); // Close the lid (move to position 180) and fade out LEDs
bool toggle_lid();
void check_auto_close(); // Check if lid should auto-close

#endif