#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

#include <Arduino.h>

// Set to 1 to enable debug output, 0 to disable
#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
  // Safe debug macros that check if Serial is available
  // This prevents blocking when running on DC power without USB
  #define DEBUG_PRINT(...) if (Serial) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) if (Serial) Serial.println(__VA_ARGS__)
  #define DEBUG_PRINTF(...) if (Serial) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
  #define DEBUG_PRINTF(...)
#endif

#endif // DEBUG_CONFIG_H
