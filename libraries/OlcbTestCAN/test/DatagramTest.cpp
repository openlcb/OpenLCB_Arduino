//===========================================================
// DatagramTest
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

unsigned int resultcode;
unsigned int datagramCallback(uint8_t rbuf[DATAGRAM_LENGTH], unsigned int length, unsigned int from){
  // invoked when a datagram arrives
  printf("consume datagram of length %d: ",length);
  for (int i = 0; i<length; i++) printf("%x ", rbuf[i]);
  printf("\n");
  return resultcode;  // return pre-ordained result
}
Datagram dg(&txBuffer, datagramCallback, &link);

/**
 * This setup is just for testing
 */
void setup()
{
  // show we've started to run
  printf("Starting DatagramTest\n");
    
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

     // Demo: handle possible production of datagram from pin
     int val = inputRead();
     if (producer_pin_record != val) {
         producer_pin_record = val;
         if (producer_pin_record == 0) {
             printf("send datagram A\n");
             uint8_t* b = dg.getTransmitBuffer();
             dg.sendTransmitBuffer(5, 0xBFD);
         } else {
             printf("send datagram B\n");
             uint8_t* b = dg.getTransmitBuffer();
             
             // check that only one possible
             if (dg.getTransmitBuffer() != 0) {
                printf("error: should not have been possible to get buffer again\n");
             }
             for (int i = 0; i<15; i++) b[i] = 16+i;
             dg.sendTransmitBuffer(15, 0xBFD);
         }
     }
  }


}


// =======================================
// end of demo program, start of test code
// =======================================

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
	printf("--------------\n");

    printf("send a first datagram \n");
    dg.getTransmitBuffer();
	dg.sendTransmitBuffer(23, 0xBFF);
	doLoop(4);

	if (dg.getTransmitBuffer() == 0) {
	    printf("1st still in use, OK\n");
	} else {
	    printf("1st error: should be still in use\n");
    }	

	printf("datagram reply for another, expect it doesn't clear buffer\n");
	b.id = 0x1E111BFD;
	b.length = (uint8_t)1;
    b.data[0]=0x4c;
    queueTestMessage(&b);
	doLoop(10);
	if (dg.getTransmitBuffer() == 0) {
	    printf("2nd still in use, OK\n");
	} else {
	    printf("2nd error: should be still in use\n");
    }	
    printf("\n");

	printf("datagram reply for this, expect it clears buffer\n");
	b.id = 0x1E573BFD;
	b.length = (uint8_t)1;
    b.data[0]=0x4c;
    queueTestMessage(&b);
	doLoop(10);
	if ( dg.getTransmitBuffer() != 0) {
	    printf("OK, send 0 length datagram\n");
	    dg.sendTransmitBuffer(0, 0xBFF);
	    loop();
        printf("handle datagram ack reply\n");
        b.id = 0x1E573BFF;
        b.length = (uint8_t)1;
        b.data[0]=0x4c;
        queueTestMessage(&b);
        doLoop(10);
        printf("\n");
	} else {
	    printf("Error: still in use\n");
    }	

	printf("send long datagram to test nak reply\n");
	uint8_t* d = dg.getTransmitBuffer();
	if ( d != 0) {
        for (int i = 0; i<23; i++) d[i] = (uint8_t)(32+i);
	    dg.sendTransmitBuffer(23, 0xBFF);
	    doLoop(4);
	} else {
	    printf("Error: buffer still in use");
    }	
    printf("datagram nak reply for different node, ignore\n");
    b.id = 0x1E111BFF;
    b.length = (uint8_t)1;
    b.data[0]=0x4d;
    queueTestMessage(&b);
    doLoop(10);
    printf("datagram nak reply from different node, ignore\n");
    b.id = 0x1E6ba111;
    b.length = (uint8_t)1;
    b.data[0]=0x4d;
    queueTestMessage(&b);
    doLoop(10);
    printf("handle datagram nak reply by resending\n");
    b.id = 0x1E573BFF;
    b.length = (uint8_t)1;
    b.data[0]=0x4d;
    queueTestMessage(&b);
    doLoop(10);
    printf("handle 2nd datagram nak reply by resending again\n");
    b.id = 0x1E573BFF;
    b.length = (uint8_t)1;
    b.data[0]=0x4d;
    queueTestMessage(&b);
    doLoop(10);
	printf("final positive reply clears buffer\n");
	b.id = 0x1E573BFF;
	b.length = (uint8_t)1;
    b.data[0]=0x4c;
    queueTestMessage(&b);
	doLoop(10);
	printf("\n");

	printf("--------------\n");
	printf("Start to test receiving datagrams\n");
	printf("\n");
	
	printf("Receive single fragment datagram OK\n");
	resultcode = 0;
	b.id = 0x1D573BFD;
	b.length = (uint8_t)4;
    b.data[0]=0x40;b.data[1]=0x41;b.data[2]=0x42;b.data[3]=0x43;
    queueTestMessage(&b);
	doLoop(10);
	printf("\n");

	printf("Receive three fragment datagram OK\n");
	resultcode = 0;
	b.id = 0x1C573BFD;
	b.length = (uint8_t)4;
    b.data[0]=0x50;b.data[1]=0x41;b.data[2]=0x42;b.data[3]=0x43;
    queueTestMessage(&b);
	doLoop(10);
	b.id = 0x1C573BFD;
	b.length = (uint8_t)4;
    b.data[0]=0x60;b.data[1]=0x41;b.data[2]=0x42;b.data[3]=0x43;
    queueTestMessage(&b);
	doLoop(10);
	b.id = 0x1D573BFD;
	b.length = (uint8_t)4;
    b.data[0]=0x70;b.data[1]=0x41;b.data[2]=0x42;b.data[3]=0x43;
    queueTestMessage(&b);
	doLoop(10);
	printf("\n");

	printf("Receive single fragment datagram, return fail\n");
	resultcode = 0x1234;
	b.id = 0x1D573BFD;
	b.length = (uint8_t)4;
    b.data[0]=0x40;b.data[1]=0x41;b.data[2]=0x42;b.data[3]=0x43;
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
