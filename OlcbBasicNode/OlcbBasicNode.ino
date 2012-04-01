//==============================================================
// OlcbBasicNode
//   A prototype of a basic 4-channel OpenLCB board
// 
//   setup() at line 189 determines which are consumers and
//   which are producers
//
//   Bob Jacobsen 2010
//      based on examples by Alex Shepherd and David Harris
//==============================================================

// next line for stand-alone compile
#include <Arduino.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "logging.h"

// The following lines are needed because the Arduino environment 
// won't search a library directory unless the library is included 
// from the top level file (this file)
#include <EEPROM.h>
#include <can.h>

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
//#include "OlcbStream.h"  // suppressed stream for space
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

NodeID nodeid(2,3,4,5,6,7);    // This node's default ID

LinkControl link(&txBuffer, &nodeid);

unsigned int datagramCallback(uint8_t *rbuf, unsigned int length, unsigned int from);
//unsigned int streamRcvCallback(uint8_t *rbuf, unsigned int length);  // suppressed stream for space

Datagram dg(&txBuffer, datagramCallback, &link);
//OlcbStream str(&txBuffer, streamRcvCallback, &link);   // suppressed stream for space

/**
 * Get and put routines that 
 * use a test memory space.
 */

// next lines get "warning: only initialized variables can be placed into program memory area" due to GCC bug
extern "C" {
const prog_char configDefInfo[] PROGMEM = "<?xml version=\"1.0\"?> \
<?xml-stylesheet type=\"text/xsl\" href=\"xslt/cdi.xsl\"?> \
<cdi xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://openlcb.org/trunk/prototypes/xml/schema/cdi.xsd\"><identification><manufacturer>OpenLCB</manufacturer><model>OlcbBasicNode</model><hardwareVersion>1.0</hardwareVersion><softwareVersion>0.4</softwareVersion></identification><segment origin=\"0\" space=\"0\"><group offset=\"77\"><name>User Identification</name><description>Lets the user add his own description</description><string size=\"12\"><name>Node Name</name></string><string size=\"20\"><name>Node Description</name></string></group><group offset=\"13\" replication=\"2\"><description>Input Pin</description><group><name>Activation Event</name><eventid/></group><group><name>Inactivation Event</name><eventid/></group></group><group replication=\"2\"><description>Output Pin</description><group><name>Set Event</name><eventid/></group><group><name>Reset Event</name><eventid/></group></group></segment><segment origin=\"0\" space=\"1\"><int size=\"4\"><name>Reset</name><description>Controls reloading and clearing node memory. Board must be restarted for this to take effect.</description><map><relation><property>0xEE555EE5</property><value>(No reset)</value></relation><relation><property>0</property><value>Reset all to defaults</value></relation><relation><property>0xEE5533CC</property><value>Reset EventIDs to new defaults</value></relation></map></int><int size=\"6\"><name>NodeID</name><description>This boards NodeID. Board must be restarted for this to take effect.</description></int></segment></cdi> \
";

const prog_char SNII_const_data[] PROGMEM = "\001OpenLCB\000OlcbBasicNode\0001.0\0000.4";

}

/* *************************************************
 * EEPROM memory layout
 *     See NodeMemory.h for background info
 *
 * 0 - 3        Memory state flag
 * 4 - 5        Cycle count
 * 
 * 6 - 12       Node ID
 *
 * 13 - 13+8*8  EventID storage
 * 77 - 89      Node name (zero-term string)
 * 90 - 110     User comment (zero-term string)
 *
 *************************************************** */

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
  } else if (space == 0xFC) { // 
    // used by ADCDI for constant data
    return pgm_read_byte(SNII_const_data+address);
  } else if (space == 0xFB) { // 
    // used by ADCDI for variable data
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

extern "C" {
uint8_t protocolIdentValue[6] = {0xD5,0x40,0,0,0,0};
}

Configuration cfg(&dg, 0, &getRead, &getWrite, (void (*)())0);
//Configuration cfg(&dg, &str, &getRead, &getWrite, (void (*)())0);   // suppressed stream for space

unsigned int datagramCallback(uint8_t *rbuf, unsigned int length, unsigned int from){
  // invoked when a datagram arrives
  //logstr("consume datagram of length ");loghex(length); lognl();
  //for (int i = 0; i<length; i++) printf("%x ", rbuf[i]);
  //printf("\n");
  // pass to consumers
  return cfg.receivedDatagram(rbuf, length, from);
}

// suppressed stream for space
//unsigned int resultcode;
//unsigned int streamRcvCallback(uint8_t *rbuf, unsigned int length){
//  // invoked when a stream frame arrives
//  //printf("consume frame of length %d: ",length);
//  //for (int i = 0; i<length; i++) printf("%x ", rbuf[i]);
//  //printf("\n");
//  return resultcode;  // return pre-ordained result
//}

// Events this node can produce or consume, used by PCE and loaded from EEPROM by NM
Event events[] = {
    Event(), Event(), Event(), Event(), 
    Event(), Event(), Event(), Event() 
};
int eventNum = 8;

// output drivers
ButtonLed p14(14, LOW);
ButtonLed p15(15, LOW);
ButtonLed p16(16, LOW);
ButtonLed p17(17, LOW);

#define ShortBlinkOn   0x00010001L
#define ShortBlinkOff  0xFFFEFFFEL

long patterns[] = {
  ShortBlinkOff,ShortBlinkOn,
  ShortBlinkOff,ShortBlinkOn,
  ShortBlinkOff,ShortBlinkOn,
  ShortBlinkOff,ShortBlinkOn
};
ButtonLed* buttons[] = {&p14,&p14,&p15,&p15,&p16,&p16,&p17,&p17};

ButtonLed blue(48, LOW);  // 18 LEDuino, 48 IO, 14 IOuino
ButtonLed gold(49, LOW);  // 19 LEDuino, 49 IO, 15 IOuino

void pceCallback(int index){
  // invoked when an event is consumed; drive pins as needed
  // from index
  //
  // sample code uses inverse of low bit of pattern to drive pin all on or all off
  // (pattern is mostly one way, blinking the other, hence inverse)
  //
  buttons[index]->on(patterns[index]&0x1 ? 0x0L : ~0x0L );
}

NodeMemory nm(0);  // allocate from start of EEPROM
void store() { nm.store(&nodeid, events, eventNum); }

PCE pce(events, eventNum, &txBuffer, &nodeid, pceCallback, store, &link);

// Set up Blue/Gold configuration

BG bg(&pce, buttons, patterns, eventNum, &blue, &gold);

bool states[] = {false, false, false, false};
void produceFromPins() {
  // called from loop(), this looks at pins and 
  // and decides which events to fire.
  // with pce.produce(i);
  // The first event of each pair is sent on button down,
  // and second on button up.
  for (int i = 0; i<eventNum/2; i++) {
    if (states[i] != buttons[i*2]->state) {
      states[i] = buttons[i*2]->state;
      if (states[i]) {
        pce.produce(i*2);
      } else {
        pce.produce(i*2+1);
      }
    }
  }
}

/**
 * Setup does initial configuration
 */
void setup()
{
  // set up serial comm; may not be space for this!
  //delay(250);Serial.begin(BAUD_RATE);logstr("\nOlcbBasicNode\n");
  
  // read OpenLCB from EEPROM
  //nm.forceInitAll(); // uncomment if need to go back to initial EEPROM state
  nm.setup(&nodeid, events, eventNum);  
  
  // set event types, now that IDs have been loaded from configuration
  for (int i=0; i<eventNum/2; i++) {
      pce.newEvent(i,true,false); // produce, consume
  }
  for (int i=eventNum/2; i<eventNum; i++) {
      pce.newEvent(i,false,true); // produce, consume
  }
  
  // Init protocol blocks
  PIP_setup(&txBuffer, &link);
  SNII_setup((uint8_t)sizeof(SNII_const_data), &txBuffer, &link);

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
  bool handled = false;
  if (rcvFramePresent) {
    // blink blue to show that the frame was received
    blue.blink(0x1);
    // see if recieved frame changes link state
    handled = link.receivedFrame(&rxBuffer);
  }

  // if link is initialized, higher-level operations possible
  if (link.linkInitialized()) {
     // if frame present, pass to handlers
     if (rcvFramePresent && rxBuffer.isMsgForHere(link.getAlias())) {
        handled |= pce.receivedFrame(&rxBuffer);
        handled |= dg.receivedFrame(&rxBuffer);
        //handled |= str.receivedFrame(&rxBuffer); // suppressed stream for space
        handled |= PIP_receivedFrame(&rxBuffer);
        handled |= SNII_receivedFrame(&rxBuffer);
        if (!handled && rxBuffer.isAddressedMessage()) link.rejectMessage(&rxBuffer);
     }
     // periodic processing of any state changes
     pce.check();
     dg.check();
     //str.check();  // suppressed stream for space
     cfg.check();
     bg.check();
     PIP_check();
     SNII_check();
     produceFromPins();
  } else {
    // link not up, but continue to show indications on blue and gold
    blue.process();
    gold.process();
  }

}


