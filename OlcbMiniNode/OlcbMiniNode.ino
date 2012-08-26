//==============================================================
// OlcbMiniNode
//   A prototype of a basic 4-channel OpenLCB board
//   with minimal protocol support:  Just PIP, basic SNII and RB
//
// 
//   setup() at line 189 determines which are consumers and
//   which are producers
//
//   Bob Jacobsen 2012
//      based on OlcbBasicNode
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

#include "OlcbCommonVersion.h"
#include "NodeID.h"

// init for serial communications if used
#define         BAUD_RATE       57600

NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 

// Define pins
// BLUE is 18 LEDuino; others defined by board (48 IO, 14 IOuino)
#define BLUE 48

// GOLD is 19 LEDuino; others defined by board (49 IO, 15 IOuino)
#define GOLD 49

/**
 * Get and put routines that 
 * use a test memory space.
 */

// next lines get "warning: only initialized variables can be placed into program memory area" due to GCC bug
extern "C" {
const prog_char configDefInfo[] PROGMEM = {
};

const prog_char SNII_const_data[] PROGMEM = "\001OpenLCB\000OlcbMiniNode\0001.0\000" OlcbCommonVersion ; // last zero in double-quote

} // end extern "C"

#define SNII_var_data 94
#define SNII_var_offset 20

#define OLCB_NO_DATAGRAM
#define OLCB_NO_STREAM
#define OLCB_NO_MEMCONFIG
#define OLCB_NO_CDI

#include "OlcbArduinoCAN.h"

/* *************************************************
 * EEPROM memory layout
 *     See NodeMemory.h for background info
 *
 * Internal data, not to be reset by user:
 *     0 - 3        Memory state flag
 *     4 - 5        Cycle count
 *     6 - 11       Node ID
 *
 * User configuration data:
 *     12 - 12+8*sizeof(Event)  EventID storage
 * 
 *     94 - 113     Node name (zero-term string, 20 bytes total)
 *     114 - 136     User comment (zero-term string, 24 bytes total)
 *
 *     12+150 - 12+150+8*sizeof(Event)  Event description strings
 *
 *************************************************** */

#define LAST_EEPROM 12+150+8*sizeof(Event)

const uint8_t getRead(uint32_t address, int space) {
  if (space == 0xFF) {
    // Configuration definition information
    return pgm_read_byte(configDefInfo+address);
  } else if (space == 0xFE) {
    // All memory reads from RAM starting at first location in this program
    return *(((uint8_t*)&rxBuffer)+address);
  } else if (space == 0xFD) {
    // Configuration space is entire EEPROM
    return EEPROM.read(address);
  } else if (space == 0xFC) { // 
    // used by ADCDI/SNII for constant data
    return pgm_read_byte(SNII_const_data+address);
  } else if (space == 0xFB) { // 
    // used by ADCDI/SNII for variable data
    return EEPROM.read(SNII_var_data+address);
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
uint8_t protocolIdentValue[6] = {0x97,0x10,0x00,0,0,0};
    // PIP, P/C, ident, teach/learn, 
    // SNIP
}

// Events this node can produce or consume, used by PCE and loaded from EEPROM by NM
Event events[] = {
    Event(), Event(), Event(), Event(), 
    Event(), Event(), Event(), Event() 
};
int eventNum = 8;

// output drivers
// 14, 15, 16, 17 for LEDuino with standard shield
// 16, 17, 18, 19 for IOduino to clear built-in blue and gold
ButtonLed pA(16, LOW); 
ButtonLed pB(17, LOW);
ButtonLed pC(18, LOW);
ButtonLed pD(19, LOW);

#define ShortBlinkOn   0x00010001L
#define ShortBlinkOff  0xFFFEFFFEL

long patterns[] = {
  ShortBlinkOff,ShortBlinkOn,
  ShortBlinkOff,ShortBlinkOn,
  ShortBlinkOff,ShortBlinkOn,
  ShortBlinkOff,ShortBlinkOn
};
ButtonLed* buttons[] = {&pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD};

ButtonLed blue(BLUE, LOW);
ButtonLed gold(GOLD, LOW);

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

BG bg(&pce, buttons, patterns, eventNum, &blue, &gold, &txBuffer);

bool states[] = {false, false, false, false};
void produceFromInputs() {
  // called from loop(), this looks at changes in input pins and 
  // and decides which events to fire
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
  nm.setup(&nodeid, events, eventNum, (uint8_t*) 0, (uint16_t)0, (uint16_t)LAST_EEPROM);  
  
  // set event types, now that IDs have been loaded from configuration
  for (int i=0; i<eventNum/2; i++) {
      pce.newEvent(i,true,false); // produce, consume
  }
  for (int i=eventNum/2; i<eventNum; i++) {
      pce.newEvent(i,false,true); // produce, consume
  }
  
  Olcb_setup();
}

void loop() {
    bool activity = Olcb_loop();
    if (activity) {
        // blink blue to show that the frame was received
        blue.blink(0x1);
    }
    if (OpenLcb_can_active) {
        gold.blink(0x1);
        OpenLcb_can_active = false;
    }
    // handle the status lights  
    blue.process();
    gold.process();
}


