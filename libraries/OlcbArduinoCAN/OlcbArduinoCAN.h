/** **********************************************
*  Provide core implementation for sketches using
*  the OlcbCommonCAN and OlcbArduinoCAN libraries.
*
*  This .h file should included in the main sketch
*  file _only_.
*
*  Conditional compilation symbols:
*   OLCB_USE_STREAMS   - #define to provide basic stream support
*   OLCB_USE_BLUE_GOLD - #define to include support for blue/gold buttons
*                                and programming; requires BLUE and GOLD
*                                to be defined as pin numbers
*
*
*/

// OpenLCB definitions
#include "OpenLcbCanInterface.h"
#include "OpenLcbCanBuffer.h"
#include "NodeID.h"
#include "EventID.h"
#include "Event.h"

// specific OpenLCB implementations
#include "LinkControl.h"
#include "Datagram.h"
#include "OlcbStream.h"  // suppress stream for space
#include "Configuration.h"
#include "NodeMemory.h"
#include "PCE.h"
#include "PIP.h"
#include "SNII.h"
#include "BG.h"
#include "ButtonLed.h"

OpenLcbCanBuffer     rxBuffer;	// CAN receive buffer
OpenLcbCanBuffer     txBuffer;	// CAN send buffer
OpenLcbCanBuffer*    ptxCAN;

LinkControl link(&txBuffer, &nodeid);

unsigned int datagramCallback(uint8_t *rbuf, unsigned int length, unsigned int from);
unsigned int streamRcvCallback(uint8_t *rbuf, unsigned int length);  // suppress stream for space

Datagram dg(&txBuffer, datagramCallback, &link);
OlcbStream str(&txBuffer, streamRcvCallback, &link);   // suppress stream for space


//Configuration cfg(&dg, 0, &getRead, &getWrite, (void (*)())0);
Configuration cfg(&dg, &str, &getRead, &getWrite, (void (*)())0);   // suppress stream for space

extern PCE pce;
extern BG bg;

unsigned int datagramCallback(uint8_t *rbuf, unsigned int length, unsigned int from){
  // invoked when a datagram arrives
  //logstr("consume datagram of length ");loghex(length); lognl();
  //for (int i = 0; i<length; i++) printf("%x ", rbuf[i]);
  //printf("\n");
  // pass to consumers
  return cfg.receivedDatagram(rbuf, length, from);
}

// suppress stream for space
unsigned int resultcode = 1;  // dummy temp value
unsigned int streamRcvCallback(uint8_t *rbuf, unsigned int length){
  return resultcode;  // return pre-ordained result
}




// invoke from setup()
void Olcb_setup() {
  // Init protocol blocks
  PIP_setup(&txBuffer, &link);
  SNII_setup((uint8_t)sizeof(SNII_const_data), 20, &txBuffer, &link);

  // Initialize OpenLCB CAN connection
  OpenLcb_can_init();
  
  // Initialize OpenLCB CAN link controller
  link.reset();
}




void produceFromInputs();

// invoke from loop()
// returns true is a frame was processed, to allow indicating activity
bool Olcb_loop() {  
  // check for input frames, acquire if present
  bool rcvFramePresent = OpenLcb_can_get_frame(&rxBuffer);
  
  // process link control first
  link.check();
  bool handled = false;  // start accumulating whether it was processed or skipped
  if (rcvFramePresent) {
    // see if recieved frame changes link state
    handled = link.receivedFrame(&rxBuffer);
  }

  // if link is initialized, higher-level operations possible
  if (link.linkInitialized()) {
     // if frame present, pass to handlers
     if (rcvFramePresent && rxBuffer.isForHere(link.getAlias())) {
        handled |= pce.receivedFrame(&rxBuffer);
        handled |= dg.receivedFrame(&rxBuffer);
        handled |= str.receivedFrame(&rxBuffer); // suppress stream for space
        handled |= PIP_receivedFrame(&rxBuffer);
        handled |= SNII_receivedFrame(&rxBuffer);
        if (!handled && rxBuffer.isAddressedMessage()) link.rejectMessage(&rxBuffer);
     }
     // periodic processing of any internal state change needs
     pce.check();
     dg.check();
     str.check();  // suppress stream for space
     cfg.check();
     bg.check();
     PIP_check();
     SNII_check();
     produceFromInputs();
  }
  return rcvFramePresent;
}

 