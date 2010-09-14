//===========================================================
// StreamTest
//   Tests for OpenLCB Stream support
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
#include "OlcbStream.h"
#include "Configuration.h"
#include "NodeMemory.h"

OpenLcbCanBuffer     rxBuffer;	// CAN receive buffer
OpenLcbCanBuffer     txBuffer;	// CAN send buffer
OpenLcbCanBuffer*    ptxCAN;

NodeID nodeid(2,3,4,5,6,7);    // This node's ID

LinkControl link(&txBuffer, &nodeid);

unsigned int datagramCallback(uint8_t rbuf[DATAGRAM_LENGTH], unsigned int length, unsigned int from);
unsigned int rcvCallback(uint8_t *rbuf, unsigned int length);

/**
 * Get and put routines that 
 * use a test memory space.
 */
uint8_t test_mem[200];

const uint8_t getRead(uint32_t address, int space) {
    return *(test_mem+address);
}
void getWrite(uint32_t address, int space, uint8_t val) {
    *(test_mem+address) = val;
}
void restart() {printf("restart called\n");}

Datagram dg(&txBuffer, datagramCallback, &link);
OlcbStream str(&txBuffer, rcvCallback, &link);
Configuration cfg(&dg, &str, &getRead, &getWrite, &restart);

unsigned int datagramCallback(uint8_t rbuf[DATAGRAM_LENGTH], unsigned int length, unsigned int from){
  // invoked when a datagram arrives
  printf("consume datagram of length %d: ",length);
  for (int i = 0; i<length; i++) printf("%x ", rbuf[i]);
  printf("\n");
  // pass to consumers
  cfg.receivedDatagram(rbuf, length, from);
  
  return 0;  // return pre-ordained result
}

unsigned int resultcode;
unsigned int rcvCallback(uint8_t *rbuf, unsigned int length){
  // invoked when a stream frame arrives
  printf("consume frame of length %d: ",length);
  for (int i = 0; i<length; i++) printf("%x ", rbuf[i]);
  printf("\n");
  return resultcode;  // return pre-ordained result
}

NodeMemory nm(0);


/**
 * This setup is just for testing
 */
void setup()
{
  // show we've started to run
  printf("Starting ConfigurationTest\n");
    
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
     // if frame present, pass to frame handlers
     if (rcvFramePresent) {
        dg.receivedFrame(&rxBuffer);
        str.receivedFrame(&rxBuffer);
     }
     // periodic processing of any datagram frames
     dg.check();
     str.check();
     cfg.check();
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


	printf("Single fragment datagram for get config\n");
	// [1D6baBFD] 20 80 01 02 03 04 02
	resultcode = 0;
	b.id = 0x1D285BFD;
	b.length = (uint8_t)7;
    b.data[0]=0x20;b.data[1]=0x80; // header
    b.data[6]=0x02;  // count
    queueTestMessage(&b);
	doLoop(10);
	printf("   Ack to config reply datagram\n");
	b.id = 0x1E285BFD;
	b.length = (uint8_t)1;
    b.data[0]=0x4c;
    queueTestMessage(&b);
	doLoop(10);
	printf("\n");
    	
	printf("Second single fragment datagram for get config\n");
	resultcode = 0;
	b.id = 0x1D285BFD;
	b.length = (uint8_t)7;
    b.data[0]=0x20;b.data[1]=0x80; // header
    b.data[6]=0x02;  // count
    queueTestMessage(&b);
	doLoop(10);
	printf("   Ack to config reply datagram\n");
	b.id = 0x1E285BFD;
	b.length = (uint8_t)1;
    b.data[0]=0x4c;
    queueTestMessage(&b);
	doLoop(10);
	printf("\n");
    	
	printf("Single fragment datagram for reset\n");
	resultcode = 0;
	b.id = 0x1D285BFD;
 	b.length = (uint8_t)2;
    b.data[0]=0x20;b.data[1]=(0x2<<6)|(0xA<<2)|(1); // header
    queueTestMessage(&b);
	doLoop(10);
	printf("\n");

	printf("Single fragment datagram for short write\n");
	resultcode = 0;
	b.id = 0x1D285BFD;
	b.length = (uint8_t)8;
    b.data[0]=0x20;b.data[1]=0x23; // header
    b.data[2]=0x00;b.data[3]=0x00;b.data[4]=0x00;b.data[5]=0x03; // address
    b.data[6]=0x44;b.data[7]=0x55;  // count
    queueTestMessage(&b);
	doLoop(10);
	printf("\n");

	printf("Single fragment datagram for short read\n");
	resultcode = 0;
	b.id = 0x1D285BFD;
	b.length = (uint8_t)7;
    b.data[0]=0x20;b.data[1]=0x61; // header
    b.data[2]=0x00;b.data[3]=0x00;b.data[4]=0x00;b.data[5]=0x00; // address
    b.data[6]=0x08;  // count
    queueTestMessage(&b);
	doLoop(10);
	printf("   Ack to reply datagram\n");
	b.id = 0x1E285BFD;
	b.length = (uint8_t)1;
    b.data[0]=0x4c;
    queueTestMessage(&b);
	doLoop(10);
	printf("\n");

	printf("test ends\n");
}

