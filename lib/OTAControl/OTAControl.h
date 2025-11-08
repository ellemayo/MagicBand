#ifndef OTA_CONTROL_H
#define OTA_CONTROL_H

#include <Arduino.h>
#include <ArduinoOTA.h>

// OTA Configuration
#define OTA_HOSTNAME "MagicBand"
#define OTA_PASSWORD "magicband2025"  // Change this to your preferred password
#define OTA_PORT 3232  // Default OTA port

// Function declarations
void setup_ota();
void loop_ota();

#endif // OTA_CONTROL_H
