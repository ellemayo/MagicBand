

#include <ServoControl.h>
#include <LEDControl.h>
#include <DebugConfig.h>

int servo_pos = 180;
Servo myservo;  // create servo object to control a servo
bool lid_is_open = false; // track lid state (starts closed at position 180)
unsigned long last_lid_open_time = 0; // track when lid was opened

void setup_servo() {
  // Connect Servo
	// Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(SERVO_PIN);  // attaches the servo on pin 18 to the servo object
  myservo.write(servo_pos);
}

// Open the lid (move from position 180 to 0)
// NOTE: This operation is intentionally blocking as the physical servo
// movement takes time. Takes approximately 2.7 seconds with 15ms per step.
void open_lid() {
  if (servo_pos == 0) {
    DEBUG_PRINTLN("Lid is already open");
    return; // Already open
  }
  
  DEBUG_PRINTLN("Opening lid...");
  for (servo_pos = 180; servo_pos > 0; servo_pos -= 1) {
    myservo.write(servo_pos);
    delay(SERVO_STEP_DELAY);
  }
  servo_pos = 0;
  lid_is_open = true;
  last_lid_open_time = millis(); // Record when we opened the lid
  DEBUG_PRINTLN("Lid opened");
}

// Close the lid (move from position 0 to 180) and fade out LEDs
// NOTE: This operation is intentionally blocking as the physical servo
// movement takes time. Takes approximately 2.7 seconds with 15ms per step.
void close_lid() {
  if (servo_pos == 180) {
    DEBUG_PRINTLN("Lid is already closed");
    return; // Already closed
  }
  
  DEBUG_PRINTLN("Closing lid...");
  for (servo_pos = 0; servo_pos < 180; servo_pos += 1) {
    myservo.write(servo_pos);
    delay(SERVO_STEP_DELAY);
  }
  servo_pos = 180;
  lid_is_open = false;
  DEBUG_PRINTLN("Lid closed");
  
  // Fade out the LEDs after lid is closed
  delay(SERVO_CLOSE_PAUSE);
  fade_out_leds();
}

bool toggle_lid() {
  if (lid_is_open) {
    // Lid is open, close it
    close_lid();
  } else {
    // Lid is closed, open it
    open_lid();
  }
  
  return lid_is_open;
}

// Check if lid should auto-close and turn off lights
void check_auto_close() {
  if (lid_is_open && (millis() - last_lid_open_time >= AUTO_CLOSE_TIMEOUT)) {
    DEBUG_PRINTLN("Auto-closing lid after inactivity timeout");
    close_lid(); // This will close the lid and fade out LEDs
  }
}