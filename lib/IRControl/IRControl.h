#ifndef IR_CONTROL_H
#define IR_CONTROL_H

#include <Arduino.h>
// Forward declarations - don't include IRremote.hpp in header to avoid multiple definitions

// IR Receiver Pin Configuration
// Default: GPIO14
// If testing for damaged GPIO, try: 13, 15, 27, 26, 25, 33, 32
// Avoid: 0,2,12,15 (strapping pins), 6-11 (flash), 34-39 (input-only)
#define IR_RECEIVE_PIN 13

// Wand IDs
#define WAND_1 20451
#define WAND_2 22171
#define WAND_3 22752

// MagiQuest protocol for IR
#define MAGIQUEST_PERIOD     1150
#define MAGIQUEST_MARK_ZERO  280
#define MAGIQUEST_SPACE_ZERO 850
#define MAGIQUEST_MARK_ONE   580
#define MAGIQUEST_SPACE_ONE  600
//#define MAGIQUEST_BITS 56

// we use an alternate `decode_type` for our IRremote decoding
//#define MAGIQUEST 11

// The magiquest payload is a bit different from the
// standard IRremote payload
union magiquest {
  uint64_t llword;
  uint8_t    byte[8];
  uint32_t  lword[2];
  struct {
    uint16_t magnitude;
    uint32_t wand_id;
    uint8_t  padding;
    uint8_t  scrap;
  } cmd ;
} ;

// hoist IRremote DEFs into our namespace for quick compatibility
#define ERR 0
#define DECODED 1

// Use analog 0 as our receiver
//int recv_pin = A0;

// Results as read from the IR sensor. We need to run this through
// the decodeMagiQuest function to get useful data.
//decode_results results;
extern magiquest data;  // External declaration - actual definition is in IRControl.cpp

void setup_ir();
uint32_t loop_ir();

// Function Definitions
int32_t  decodeMagiQuest(magiquest *mdata);

#endif