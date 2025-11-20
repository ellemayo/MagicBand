#include <IRControl.h>
#include <IRremote.hpp>  // Include the full IRremote library only in the .cpp file
#include <DebugConfig.h>

// Define the global variable here (declared as extern in the header)
magiquest data;


void setup_ir() {
  // turn on IR receiver
  //IrReceiver.enableIRIn();
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  //IrReceiver.begin(3);
}

uint32_t loop_ir() {
  uint32_t wand_id = 0;

  // Wait and decode
  //if (IrReceiver.decode(&results)) {
  if (IrReceiver.decode()) {
 
    // translate the bit stream into something we can use
    // to understand a MagiQuest wand
    if (decodeMagiQuest(&data) == DECODED) {
      wand_id = data.cmd.wand_id;
      
      DEBUG_PRINT("wand_id: ");
      DEBUG_PRINTLN(data.cmd.wand_id);
      //DEBUG_PRINT("magnitude: ");
      //DEBUG_PRINTLN(data.cmd.magnitude);
    } else {
      // Invalid IR signal - explicitly clear wand_id
      DEBUG_PRINTLN("IR decode error - invalid MagiQuest signal");
      wand_id = 0;
    }

    // keep receiving data 
    IrReceiver.resume(); 
  }
  
  return wand_id;
}

/*
 * This decodeMagiQuest method cribbed from mpflaga (https://github.com/mpflaga/Arduino-IRremote) 
 * mode of the Arduino IRremote library. Excised and updated to work with current IRremote
 * library.
 * 
 * https://github.com/mpflaga/Arduino-IRremote/blob/master/IRremote.cpp
 * 
 */
int32_t  decodeMagiQuest(magiquest *mdata) {
  magiquest data;
  data.llword = 0;

  int16_t offset = 1;
  uint16_t mark_;
  uint16_t space_;
  uint8_t multiple_;

  if (IrReceiver.decodedIRData.rawlen < 2 * MAGIQUEST_BITS) {
    return ERR;
  }

  while (offset + 1 < IrReceiver.decodedIRData.rawlen) {
    mark_ = IrReceiver.irparams.rawbuf[offset];
    space_ = IrReceiver.irparams.rawbuf[offset+1];
    multiple_ = space_ / mark_;
    // it is either 25% + 75% or 50% + 50%

    if (MATCH_MARK(space_ + mark_, MAGIQUEST_PERIOD)) {
      if (multiple_ > 1) {
        data.llword <<= 1;
      } else {
        data.llword = (data.llword << 1) | 1;
      }
    } else {
      return ERR;
    }
    offset++;
    offset++;
  }
  // Success
  
  int16_t bits = (offset + 1) / 2;
  if (bits < 12) {
    bits = 0;
    return ERR;
  }
  //results->magiquestMagnitude = data.cmd.magnitude;
  IrReceiver.decodedIRData.decodedRawData = data.cmd.wand_id;
  //IrReceiver.decodedIRData.protocol = MAGIQUEST;

  // fill in our magiquest struct
  mdata->cmd.magnitude = data.cmd.magnitude;
  mdata->cmd.wand_id = data.cmd.wand_id;
  return DECODED;
}