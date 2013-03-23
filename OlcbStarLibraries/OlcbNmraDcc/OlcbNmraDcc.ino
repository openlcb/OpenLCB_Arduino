//==============================================================
// OlcbNmraDcc
//   A prototype of an OpenLCB to NMRA DCC bridge
//
//  Note:  The NmraDcc library requires pin definitions that are
//         not available on all board types. Best used with a 
//         328-style Arduino
// 
//   Bob Jacobsen 2010
//==============================================================
#include <arduino.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "logging.h"

#include <NmraDcc.h>
#include <ButtonLed.h>

NmraDcc  Dcc ;
DCC_MSG  Packet ;

// Called by the NmraDcc library when a DCC ACK needs to be sent
void notifyCVAck(void)
{
 // we don't handle programming, so don't do anything here
}

// The following lines are needed because the Arduino environment 
// won't search a library directory unless the library is included 
// from the top level file (this file)
#include <EEPROM.h>
#include <can.h>

class foo{};

// init for serial communications
//#define         BAUD_RATE       115200
#define         BAUD_RATE       57600
//#define         BAUD_RATE       19200

// OpenLCB definitions
#include "OpenLcbCanInterface.h"
#include "OpenLcbCanBuffer.h"
#include "NodeID.h"
#include "EventID.h"
#include "Event.h"

// specific OpenLCB implementations
#include "LinkControl.h"
#include "Datagram.h"
#include "OlcbStream.h"
#include "Configuration.h"
#include "NodeMemory.h"
#include "PCE.h"
#include "BG.h"
#include "ButtonLed.h"

OpenLcbCanBuffer     rxBuffer;	// CAN receive buffer
OpenLcbCanBuffer     txBuffer;	// CAN send buffer
OpenLcbCanBuffer*    ptxCAN;

NodeID nodeid(2,3,4,5,6,0x41);    // This node's default ID

LinkControl link(&txBuffer, &nodeid);

/**
 * Get and put routines that 
 * use a test memory space.
 */
prog_char configDefInfo[] PROGMEM = "OlcbNmraDcc CDI"; // null terminated string

const uint8_t getRead(uint32_t address, int space) {
  if (space == 0xFF) {
    // Configuration definition information
    return pgm_read_byte(configDefInfo+address);
  } else if (space == 0xFE) {
    // All memory
    return *(((uint8_t*)&rxBuffer)+address);
  } else if (space == 0xFD) {
    // Configuration space
    return EEPROM.read(address);
  } else {
    // unknown space
    return 0; 
  }
}
void getWrite(uint32_t address, int space, uint8_t val) {
  if (space == 0xFE) {
    // All memory
    *(((uint8_t*)&rxBuffer)+address) = val;
  } else if (space == 0xFD) {
    // Configuration space
    EEPROM.write(address, val);
  } 
  // all other spaces not written
}

ButtonLed blue(18, LOW);
ButtonLed gold(19, LOW);

/**
 * Setup does initial configuration
 */
void setup()
{
  // set up serial comm; may not be space for this!
  //delay(250);Serial.begin(BAUD_RATE);logstr("\nOlcbConfigureTest\n");
  
  // read OpenLCB from EEPROM
  //nm.forceInitAll(); // uncomment if need to go back to initial EEPROM state
  //nm.setup(&nodeid, events, eventNum);  
  
  // set event types, now that IDs have been loaded from configuration
  //for (int i=0; i<4; i++) {
      //pce.newEvent(i,true,false); // produce, consume
  //}
  //for (int i=4; i<8; i++) {
      //pce.newEvent(i,false,true); // produce, consume
  //}
  
  // Initialize OpenLCB CAN connection
  OpenLcb_can_init();
  
  // Initialize OpenLCB CAN link controller
  link.reset();

  // Call the main DCC Init function to enable the DCC Receiver
  Dcc.init( MAN_ID_DIY, 10, FLAGS_OUTPUT_ADDRESS_MODE | FLAGS_DCC_ACCESSORY_DECODER, 33 );
}

void loop() {

  // check for input frames, acquire if present
  bool rcvFramePresent = OpenLcb_can_get_frame(&rxBuffer);
  
  // process link control first
  link.check();
  if (rcvFramePresent) {
    // blink blue to show that the frame was received
    blue.blink(0x1);
    // see if recieved frame changes link state
    link.receivedFrame(&rxBuffer);
  }

  // if link is initialized, higher-level operations possible
  if (link.linkInitialized()) {
     // if frame present, pass to handlers
     if (rcvFramePresent) {
        //pce.receivedFrame(&rxBuffer);
        //dg.receivedFrame(&rxBuffer);
        //str.receivedFrame(&rxBuffer);
     }
     // periodic processing of any state changes
     //pce.check();
     //dg.check();
     //str.check();
     //cfg.check();
     //bg.check();
     //produceFromPins();
  } else {
    // link not up, but continue to show indications on blue and gold
    blue.process();
    gold.process();
  }
  // process DCC input
  Dcc.process();
}

void notifyDccAccState( uint16_t Addr, uint16_t BoardAddr, uint8_t OutputAddr, uint8_t State )
{
    // insert proper values in bottom four bytes
  Event dccEvent(0x11,0x12,0x13,0x14, (byte)(Addr >> 8), (byte)(Addr & 0x00FF), OutputAddr, State);
  txBuffer.setProducerIdentified(&dccEvent);
  OpenLcb_can_queue_xmt_wait(&txBuffer);  // wait until buffer queued, but OK due to earlier check
}

void notifyDccSigState( uint16_t Addr, uint8_t OutputIndex, uint8_t State)
{
    // insert proper values in bottom four bytes
  Event dccEvent(0x11,0x12,0x13,0x14, (byte)(Addr >> 8), (byte)(Addr & 0x00FF), OutputIndex, State);
  txBuffer.setProducerIdentified(&dccEvent);
  OpenLcb_can_queue_xmt_wait(&txBuffer);  // wait until buffer queued, but OK due to earlier check
}

