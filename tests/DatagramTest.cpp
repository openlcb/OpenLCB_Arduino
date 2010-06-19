//===========================================================
// DatagramTests
//   Tests for OpenLCB Datagrams support
// 
//   Bob Jacobsen 2010 
//===========================================================
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "logging.h"

int inputRead() {
    return 1;
}

// demo I/O pins
#define INPUT_PIN 9
#define OUTPUT_PIN 14
int producer_pin_record;

// OpenLCB definitions
#include "OpenLcbCanInterface.h"
#include "OpenLcbCanBuffer.h"
#include "NodeID.h"
#include "EventID.h"

// specific OpenLCB implementations
#include "LinkControl.h"
#include "Datagram.h"

OpenLcbCanBuffer     rxBuffer;	// CAN receive buffer
OpenLcbCanBuffer     txBuffer;	// CAN send buffer
OpenLcbCanBuffer*    ptxCAN;

NodeID nodeid(2,3,4,5,6,7);    // This node's ID

LinkControl link(&txBuffer, &nodeid);

void datagramCallback(int index){
  // invoked when a datagram arrives
  printf("consume %d\n",index);
}
Datagram dg(&txBuffer, &nodeid, datagramCallback);

/**
 * This setup is just for testing
 */
void setup()
{
  // show we've started to run
  printf("Starting CanMrrlcbTest\n");
    
  // Initialize OpenLCB CAN connection
  OpenLcb_can_init();
  
  // Initialize OpenLCB CAN link controller
  link.reset();
}

void loop() {
  // check for input frames, acquire if present
  bool rcvFramePresent = OpenLcb_can_get_frame(&rxBuffer);
  
  // process link control first
  link.check();
  if (rcvFramePresent) {
    // received a frame, ask if changes link state
    link.receivedFrame(&rxBuffer);
  }

  // if link is initialized, higher-level operations possible
  if (link.linkInitialized()) {
     // if frame present, pass to Datagram handler
     if (rcvFramePresent) {
        dg.receivedFrame(&rxBuffer);
     }
     // periodic processing of any datagram frames
     dg.check();

     // Demo: handle possible production of events from pin
     int val = inputRead();
     if (producer_pin_record != val) {
         producer_pin_record = val;
         if (producer_pin_record == 0) {
             printf("send datagram A\n");
             //p.produce(0);
         } else {
             printf("send datagram B\n");
             //p.produce(1);
         }
     }
  }


}

void doLoop(int n) {
	for (int i = 0; i < n; i++) {
        loop();
    }
}

OpenLcbCanBuffer b;

int main( int argc, const char* argv[] )
{
    // run the code for a test
	printf("--------------\n");
	setup();
	printf("setup done\n");
	doLoop(1000);  // long enough for timeout
	printf("one second done\n\n");


	printf("queue short datagram, expect 1 reply\n");
	b.id = 0x1824F00F;
	b.length = (uint8_t)8;
	b.data[0]=1;b.data[1]=2;b.data[2]=3;b.data[3]=4;b.data[4]=5;b.data[5]=6;b.data[6]=7;b.data[7]=8;
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");



	printf("test ends\n");
}

// to test (messages in JMRI format)
//    send a CIM frame which should get a RIM:  [110036ba]
//    then a RIM which should restart sequence: [17fff6ba]

// (these need to be redone)
//    send a Verify Node frame of [180Af00f] 2 3 4 5 6 7
//    send a Request Consumers frame of [1824F00F] 1 2 3 4 5 6 7 8
//    send a Request Producers frame of [1828F00F] 8 7 6 5 4 3 2 1
//    send a Request Events frame of [182BF00F] 2 3 4 5 6 7

//    produce an event with [182DF00F] 8 7 6 5 4 3 2 1
