#include "OTAControl.h"
#include <DebugConfig.h>
#include <LEDControl.h>

/**
 * Initialize OTA (Over-The-Air) update functionality
 * This allows wireless firmware updates without USB connection
 */
void setup_ota() {
  DEBUG_PRINTLN("Initializing OTA updates...");
  
  // Set OTA hostname - this is how you'll identify the device
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  
  // Set OTA password for security
  ArduinoOTA.setPassword(OTA_PASSWORD);
  
  // Set OTA port
  ArduinoOTA.setPort(OTA_PORT);
  
  // OTA callbacks for visual feedback and debugging
  
  // When OTA update starts
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    
    DEBUG_PRINTLN("OTA Update Started: " + type);
    
    // Visual feedback - turn all LEDs white during update
    fill_solid(leds, NUM_LEDS, CRGB::White);
    FastLED.setBrightness(50);
    FastLED.show();
  });
  
  // When OTA update ends
  ArduinoOTA.onEnd([]() {
    DEBUG_PRINTLN("\nOTA Update Complete!");
    
    // Visual feedback - flash green for success
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.setBrightness(100);
    FastLED.show();
    delay(1000);
  });
  
  // During OTA update - show progress
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    unsigned int percent = (progress / (total / 100));
    
    DEBUG_PRINT("OTA Progress: ");
    DEBUG_PRINT(percent);
    DEBUG_PRINTLN("%");
    
    // Visual feedback - pulse LEDs to show activity
    if (percent % 10 == 0) {  // Every 10%
      fill_solid(leds, NUM_LEDS, CRGB::Blue);
      FastLED.setBrightness(percent);
      FastLED.show();
    }
  });
  
  // On OTA error
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_PRINT("OTA Error[");
    DEBUG_PRINT(error);
    DEBUG_PRINT("]: ");
    
    if (error == OTA_AUTH_ERROR) {
      DEBUG_PRINTLN("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      DEBUG_PRINTLN("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      DEBUG_PRINTLN("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      DEBUG_PRINTLN("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      DEBUG_PRINTLN("End Failed");
    }
    
    // Visual feedback - flash red for error
    for (int i = 0; i < 5; i++) {
      fill_solid(leds, NUM_LEDS, CRGB::Red);
      FastLED.setBrightness(100);
      FastLED.show();
      delay(200);
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      delay(200);
    }
  });
  
  // Start OTA service
  ArduinoOTA.begin();
  
  DEBUG_PRINTLN("OTA updates ready!");
  DEBUG_PRINT("Hostname: ");
  DEBUG_PRINTLN(OTA_HOSTNAME);
  DEBUG_PRINT("Port: ");
  DEBUG_PRINTLN(OTA_PORT);
  DEBUG_PRINTLN("Use PlatformIO with --upload-port flag or Arduino IDE to upload wirelessly");
}

/**
 * Handle OTA update requests
 * Must be called regularly in the main loop
 */
void loop_ota() {
  ArduinoOTA.handle();
}
