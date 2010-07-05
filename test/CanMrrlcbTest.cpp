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

// Events this node can consume and produce, used by PCE
Event events[] = {
    Event(1,2,3,4,5,6,7,8), 
    Event(17,18,19,20,21,22,23,24),
    Event(33,34,35,36,37,38,39,40), 
    Event(65,66,67,68,69,70,71,72)
};
int evtNum = 4;

void pceCallback(int index){
  // invoked when an event is consumed
  printf("consume %d\n",index);
}
PCE pce(events, evtNum, &txBuffer, &nodeid, pceCallback);

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
  
  // set event types
  pce.newEvent(0,true, false);
  pce.newEvent(1,true, false);
  pce.newEvent(2,false, true);
  pce.newEvent(3,false, true);
  
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
        pce.receivedFrame(&rxBuffer);
     }
     // periodic processing of any PCE state changes
     pce.check();

     // Demo: handle possible production of events from pin
     int val = inputRead();
     if (producer_pin_record != val) {
         producer_pin_record = val;
         if (producer_pin_record == 0) {
             printf("produce p0\n");
             pce.produce(0);
         } else {
             printf("produce p1\n");
             pce.produce(1);
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
	b.id = 0x10700285;
	b.length = 0;
	queueTestMessage(&b);
	doLoop(1000);  // long enough for timeout
	printf("\n");

	printf("trigger CIM conflict\n");
	b.id = 0x163046ba;
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
	b.data[0]=65;b.data[1]=66;b.data[2]=67;b.data[3]=68;b.data[4]=69;b.data[5]=70;b.data[6]=71;b.data[7]=72;
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");

	printf("queue Request Consumers, expect no reply when matching producer event\n");
	b.id = 0x1824F00F;
	b.length = (uint8_t)8;
	b.data[0]=17;b.data[1]=18;b.data[2]=19;b.data[3]=20;b.data[4]=21;b.data[5]=22;b.data[6]=23;b.data[7]=24;
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");

	printf("queue Request Consumers, no reply due to not match\n");
	b.id = 0x1824F00F;
	b.length = (uint8_t)8;
	b.data[0]=1;b.data[1]=12;b.data[2]=3;b.data[3]=4;b.data[4]=5;b.data[5]=6;b.data[6]=7;b.data[7]=8;
	queueTestMessage(&b);
	doLoop(100);
	printf("\n");

	printf("queue Request Producers, expect 1 reply\n");
	b.id = 0x1828F00F;
	b.length = (uint8_t)8;
	b.data[0]=17;b.data[1]=18;b.data[2]=19;b.data[3]=20;b.data[4]=21;b.data[5]=22;b.data[6]=23;b.data[7]=24;
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
    if (!original0.equals(&events[0])) printf(" *** not match at beginning\n");

	printf("Send stand-alone learn messages (2nd, then 1st sent)\n");
	pce.sendTeach(1);
	doLoop(10);
	pce.sendTeach(0);
	doLoop(10);
	printf("\n");

	printf("Mark to learn 1st, 2nd, then unmark 1st\n");
	pce.markToLearn(0, true);
	pce.markToLearn(1, true);
	pce.markToLearn(0, false);
    printf("Teach new event, expect to announce used\n");
	b.id = 0x182cf00F;
	b.length = (uint8_t)8;
	b.data[0]=0x28;b.data[1]=0x27;b.data[2]=0x26;b.data[3]=0x25;b.data[4]=0x24;b.data[5]=0x23;b.data[6]=0x22;b.data[7]=0x21;
	queueTestMessage(&b);
	doLoop(20);
	printf("queue Request Events, expect 2nd changed\n");
	b.id = 0x182BF00F;
	b.length = (uint8_t)6;
	b.data[0]=2; b.data[1]=3; b.data[2]=4; b.data[3]=5; b.data[4]=6; b.data[5]=7; 
	queueTestMessage(&b);
	doLoop(20);
	printf("\n");

	printf("test ends\n");
	
}

