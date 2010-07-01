//===========================================================
// CanMrrlcbTest
//   Developing (eventual) classes for OpenLCB
// 
//   Bob Jacobsen 2009
//      based on examples by Alex Shepherd and David Harris
//===========================================================
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

// The following line is needed because the Arduino environment 
// won't search a library directory unless the library is included 
// from the top level file (this file)
#include <CAN.h>
#include "logging.h"

int inputRead() {
    return 1;
}

// demo I/O pins
#define CONSUMER_PIN 9
#define PRODUCER_PIN 14
int producer_pin_record;

// OpenLCB definitions
#include "OpenLcbCanInterface.h"
#include "OpenLcbCanBuffer.h"
#include "NodeID.h"
#include "EventID.h"

// specific OpenLCB implementations
#include "LinkControl.h"
#include "PCE.h"

OpenLcbCanBuffer     rxBuffer;	// CAN receive buffer
OpenLcbCanBuffer     txBuffer;	// CAN send buffer
OpenLcbCanBuffer*    ptxCAN;

NodeID nodeid(2,3,4,5,6,7);    // This node's ID

LinkControl link(&txBuffer, &nodeid);

// Events this node can produce, used by PCE
Event pEvents[] = {
    Event(1,2,3,4,5,6,7,8), 
    Event(8,7,6,5,4,3,2,1)
};
int pEventNum = 2;

// Events this node can consume, used by PCE
Event cEvents[] = {
    Event(1,2,3,4,5,6,7,8), 
    Event(8,7,6,5,4,3,2,1)
};
int cEventNum = 2;

void pceCallback(int index){
  // invoked when an event is consumed
  printf("consume %d\n",index);
}
PCE p(cEvents, cEventNum, pEvents, pEventNum, &txBuffer, &nodeid, pceCallback);

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
     // if frame present, pass to PC handler
     if (rcvFramePresent) {
        p.receivedFrame(&rxBuffer);
     }
     // periodic processing of any PCE state changes
     p.check();

     // Demo: handle possible production of events from pin
     int val = inputRead();
     if (producer_pin_record != val) {
         producer_pin_record = val;
         if (producer_pin_record == 0) {
             printf("produce p0\n");
             p.produce(0);
         } else {
             printf("produce p1\n");
             p.produce(1);
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

	printf("trigger RIM conflict\n");
	b.id = 0x107006ba;
	b.length = 0;
	queueTestMessage(&b);
	doLoop(1000);  // long enough for timeout
	printf("\n");

	printf("trigger CIM conflict\n");
	b.id = 0x16304e8d;
	b.length = 0;
	queueTestMessage(&b);
	doLoop(1000);  // long enough for timeout
	printf("\n");

	printf("queue Verify Node\n");
	b.id = 0x180AF00F;
	b.length = (uint8_t)6;
	b.data[0]=2; b.data[1]=3; b.data[2]=4; b.data[3]=5; b.data[4]=6; b.data[5]=7; 
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");

	printf("queue Request Consumers, expect 1 reply\n");
	b.id = 0x1824F00F;
	b.length = (uint8_t)8;
	b.data[0]=1;b.data[1]=2;b.data[2]=3;b.data[3]=4;b.data[4]=5;b.data[5]=6;b.data[6]=7;b.data[7]=8;
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");

	printf("queue Request Consumers, no reply\n");
	b.id = 0x1824F00F;
	b.length = (uint8_t)8;
	b.data[0]=1;b.data[1]=12;b.data[2]=3;b.data[3]=4;b.data[4]=5;b.data[5]=6;b.data[6]=7;b.data[7]=8;
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");

	printf("queue Request Producers, expect 1 reply\n");
	b.id = 0x1828F00F;
	b.length = (uint8_t)8;
	b.data[0]=8;b.data[1]=7;b.data[2]=6;b.data[3]=5;b.data[4]=4;b.data[5]=3;b.data[6]=2;b.data[7]=1;
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");

	printf("queue Request Producers, no reply\n");
	b.id = 0x1828F00F;
	b.length = (uint8_t)8;
	b.data[0]=8;b.data[1]=12;b.data[2]=6;b.data[3]=5;b.data[4]=4;b.data[5]=3;b.data[6]=2;b.data[7]=1;
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");

	printf("queue Request Events, expect 4 replies\n");
	b.id = 0x182BF00F;
	b.length = (uint8_t)6;
	b.data[0]=2; b.data[1]=3; b.data[2]=4; b.data[3]=5; b.data[4]=6; b.data[5]=7; 
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");

	printf("queue Request Events, no reply\n");
	b.id = 0x182BF00F;
	b.length = (uint8_t)6;
	b.data[0]=25; b.data[1]=3; b.data[2]=4; b.data[3]=5; b.data[4]=6; b.data[5]=7; 
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");

	printf("queue P/C Event Transfer, no match\n");
	b.id = 0x182DF00F;
	b.length = (uint8_t)8;
	b.data[0]=0;b.data[1]=12;b.data[2]=6;b.data[3]=5;b.data[4]=4;b.data[5]=3;b.data[6]=2;b.data[7]=1;
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");

	printf("queue P/C Event Transfer, match event 1\n");
	b.id = 0x182DF00F;
	b.length = (uint8_t)8;
	b.data[0]=8;b.data[1]=7;b.data[2]=6;b.data[3]=5;b.data[4]=4;b.data[5]=3;b.data[6]=2;b.data[7]=1;
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");

    printf("----- start learn/teach tests ----------\n");
    Event original0(1,2,3,4,5,6,7,8);
    if (!original0.equals(&pEvents[0])) printf(" *** not match at beginning\n");

	printf("Send stand-alone learn messages\n");
	p.sendTeachC(1);
	doLoop(10);
	p.sendTeachP(0);
	doLoop(10);
	printf("\n");

	printf("Learn p0, c1\n");
	p.markToLearnP(0);
	p.markToLearnC(1);
    printf("Teach new event 0x2x\n");
	b.id = 0x182cf00F;
	b.length = (uint8_t)8;
	b.data[0]=0x28;b.data[1]=0x27;b.data[2]=0x26;b.data[3]=0x25;b.data[4]=0x24;b.data[5]=0x23;b.data[6]=0x22;b.data[7]=0x21;
	queueTestMessage(&b);
	doLoop(10);
	printf("queue Request Events, expect 1st and last changed\n");
	b.id = 0x182BF00F;
	b.length = (uint8_t)6;
	b.data[0]=2; b.data[1]=3; b.data[2]=4; b.data[3]=5; b.data[4]=6; b.data[5]=7; 
	queueTestMessage(&b);
	doLoop(10);
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
