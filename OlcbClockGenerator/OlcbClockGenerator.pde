//==============================================================
// OlcbConfigureTest
//   A prototype of a Clock-generator OpenLCB board
// 
//   setup() at line 189 determines which are consumers and
//   which are producers
//
//   Bob Jacobsen 2010
//   David Harris
//      based on examples by Alex Shepherd 
//==============================================================
#include <arduino.h>

//#define PL(x) Serial.println(x)
//#define P(x) Serial.print(x)
//#define PD(x) Serial.print(x,DEC)
#define PL(x) 
#define P(x) 
#define PD(x)

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

class foo{};

// init for serial communications
#define         BAUD_RATE       115200
//#define         BAUD_RATE       57600
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

NodeID nodeid(5,1,1,1,1,10);    // This node's default ID

LinkControl link(&txBuffer, &nodeid);


unsigned int datagramCallback(uint8_t *rbuf, unsigned int length, unsigned int from);
unsigned int streamRcvCallback(uint8_t *rbuf, unsigned int length);

Datagram dg(&txBuffer, datagramCallback, &link);
OlcbStream str(&txBuffer, streamRcvCallback, &link);

/**
 * Get and put routines that 
 * use a test memory space.
 */

prog_char configDefInfo[] PROGMEM = "OlcFastClockGenNode"; // null terminated string

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

Configuration cfg(&dg, &str, &getRead, &getWrite, (void (*)())0);

unsigned int datagramCallback(uint8_t *rbuf, unsigned int length, unsigned int from){
  // invoked when a datagram arrives
  //logstr("consume datagram of length ");loghex(length); lognl();
  //for (int i = 0; i<length; i++) printf("%x ", rbuf[i]);
  //printf("\n");
  // pass to consumers
  cfg.receivedDatagram(rbuf, length, from);
  
  return 0;  // return pre-ordained result
}

unsigned int resultcode;
unsigned int streamRcvCallback(uint8_t *rbuf, unsigned int length){
  // invoked when a stream frame arrives
  //printf("consume frame of length %d: ",length);
  //for (int i = 0; i<length; i++) printf("%x ", rbuf[i]);
  //printf("\n");
  return resultcode;  // return pre-ordained result
}

// Events this node can produce or consume, used by PCE and loaded from EEPROM by NM
Event events[] = {
    Event(),  // time event xxx.hr.min
    Event(),  // rate event xxx.00.rate
    Event(),  // rate event xxx.00.rate
    Event()  // rate event xxx.00.rate
};
int eventNum = 4;

// output drivers
ButtonLed p16(16, LOW);   // minutes
ButtonLed p17(17, LOW);   // hours
ButtonLed blue(18, LOW);   // nominate time
ButtonLed gold(19, LOW);   // teach time

#define ShortBlinkOn   0x00010001L
#define ShortBlinkOff  0xFFFEFFFEL

long patterns[] = {ShortBlinkOff};
//ButtonLed* buttons[] = {&p14,&p14,&p15,&p15,&p16,&p16,&p17,&p17};
ButtonLed* buttons[] = {&p16,&p16,&p17,&p17};

void pceCallback(int index){
  // invoked when an event is consumed; drive pins as needed
  // from index
  //
  // sample code uses inverse of low bit of pattern to drive pin all on or all off
  // (pattern is mostly one way, blinking the other, hence inverse)
  //
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
  for (int i = 0; i<2; i++) {
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



//============================================================

int mode;
boolean oldp16, oldp17, oldgold, oldblue;
int nomMode;
boolean teachMode, state;
int  rate, minutes, hours;
boolean  newTime, newRate, p;
int inc, newrate, newminutes, newhours;  // for teach and nominate modes
long divisor;
boolean next;

void sendTime(uint8_t rate, uint8_t hours, uint8_t minutes);

void processTime() {
    if( (nomMode==0) && !teachMode ) {
     if(next && (millis()%divisor)==0) {
       minutes += 1; 
       if(minutes>59) {
         minutes = 0;
         hours += 1;
         if(hours>23) hours = 0;
       }
       next = false;
       sendTime(rate, hours, minutes);
       
       Event clockEvent = Event(0x1,0x1,0x99,0x1,0x1,0x1, hours, minutes);
       txBuffer.setPCEventReport(&clockEvent);
       OpenLcb_can_queue_xmt_wait(&txBuffer);  // wait until buffer queued, but OK due to earlier check

     }
  }
  if((millis()%divisor)!=0) next = true;
}

void upDownProcess() {
 if(!teachMode && (nomMode==0)) return;
     switch(mode) {
     case 1:  // change hours
       newhours += inc;
       if(newhours>23) newhours = 0;
       if(newhours<0) newhours = 23;
       newTime = true;
       break;
     case 2: // change minutes
       newminutes += inc;
       if(newminutes>59) newminutes = 0;
       if(newminutes<0) newminutes = 59;
       newTime = true;
       break;
     case 3: // change rate
       newrate += inc*5;
       if(newrate>200) newrate = 200;
       if(newrate<1) newrate = 1;
       newRate = true;;
       break;
     }
     sendTime(newrate, newhours, newminutes);  // confusing
}



void sendTime(uint8_t rate, uint8_t hours, uint8_t minutes) {
  PD(rate);
  P("=rate, time=");
  PD(hours);
  P(":");
  PD(minutes);
  PL();
}

// interface:
//  To set time: blue; gold to select: hour, minute, rate; up/down to change; blue
//  To Teach: gold; blue to select: hour, minute, rate; up/down to change; gold
//  To nominate: blue, blue; gold to select: h,m,r; up/down; blue

void processControls() {
   p16.process();               // up
   if (oldp16 != p16.state) {
     oldp16 = p16.state;
     if (oldp16) {
       inc = -1;       // act on button down
       upDownProcess();       
     }
   }
   p17.process();               // down
   if (oldp17 != p17.state) {
     oldp17 = p17.state;
     if (oldp17) {
       inc = 1;       // act on button down
       upDownProcess();       
     }
   }
  // blue
  blue.process();
  if (oldblue != blue.state) {
    oldblue = blue.state;
    if (oldblue) {                   // Blue was down
      // check gold button state
      if (gold.state) {
                                      // if Gold is simultaneously down, send ident
//        sendIdent();
      }
      else 
      if(teachMode) {           // Teach Mode, blue steps through h,m,r 
        if(mode==0) {
          mode = 1;
          blue.on(0x000000F0);
          PL("hour");
        } 
        else if(mode==1) {
          mode = 2;
          blue.on(0x0000F0F0);
          PL("min");
        } else if(mode==2) {
          mode = 3;
          blue.on(0x00F0F0F0);
          PL("rate");
        } else {
          mode = 0;
          blue.on(0);
        }
      } 
      else if(nomMode>0) {
        if(newTime|newRate) {    // something changed, cancel mode
          if(nomMode==1) {
            if(newTime) { hours=newhours; minutes=newminutes; }
            if(newRate) { rate = newrate; divisor = 60000 / rate; } 
          } // else we are set for learn event
        }
        nomMode = 0;
        gold.on(0);
        blue.on(0);
      }
      else {    // move to next nomMode (run, setClock, nominate)
        if(nomMode==0) {
          newhours=8; newminutes=0; newrate=60; 
          newRate=newTime=false;
          nomMode = 1;
          mode = 1;
          blue.on(0x000000F0);
          gold.on(0x000000F0);
          PL("Set\nhour");
        } 
        else if(nomMode==1) {
          nomMode = 2;
          PL("Nom\ntime");
          blue.on(0x000F0F0);
        } 
        else if(nomMode==2) { 
          nomMode = 0;
          blue.on(0);
          gold.on(0);
        } 
      }
    }
  }
  gold.process();
  if (oldgold != gold.state) {
    oldgold = gold.state;
    if (oldgold) {                   // Gold is down
      gold.on(~0);
      if(!teachMode) {                // not in teaching mode
        if(nomMode==0) {               // not updating time or rate nor nominationg, go to teachMode
          teachMode = true;
          gold.on(~0);
          blue.on(0x000000F0);
          newhours=8; newminutes=0; newrate=60;
          newTime = newRate = false;
          mode=1;
          PL("teaching\nhours");
        } else {                        // step through h,m,r
          if(mode==0) {
            mode = 1;
            gold.on(0x000000F0);
            PL("hours");
          }
          else if(mode==1) {
            mode = 2;
            gold.on(0x0000F0F0);
            PL("min");
          } 
          else if(mode==2) {
            mode = 3;
            gold.on(0x00F0F0F0);
            PL("rate");
          } 
          else {
            mode = 0;
            gold.on(0);
            blue.on(0);
          }
        }
      }
      else {                         // teachMode, do it if something changed
        if(newRate) {
            PL("Teach rate");
            
        }
        else if(newTime) {
          PL("Teach time");
        }
        teachMode = false;            // cancel teachMode 
        gold.on(0);
        blue.on(0);
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
  //delay(250);Serial.begin(9600);logstr("\nOlcbClockGenerator\n");
 
  // read OpenLCB from EEPROM
  //nm.forceInitAll(); // uncomment if need to go back to initial EEPROM state
  nm.setup(&nodeid, events, eventNum);  
  
  // set event types, now that IDs have been loaded from configuration
  for (int i=0; i<2; i++) {
      pce.newEvent(i,true,false); // produce, consume
  }
//  for (int i=4; i<8; i++) {
//      pce.newEvent(i,false,true); // produce, consume
//  }
  
  // Initialize OpenLCB CAN connection
  OpenLcb_can_init();
  
  // Initialize OpenLCB CAN link controller
  link.reset();

    hours= 9;
    minutes = 0;
    rate = 60;
    divisor = 60000 / rate;
    next = true;
    digitalWrite(p16.pin,HIGH);
    digitalWrite(p17.pin,HIGH);
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
        pce.receivedFrame(&rxBuffer);
        dg.receivedFrame(&rxBuffer);
        str.receivedFrame(&rxBuffer);
     }
     // periodic processing of any state changes
     pce.check();
     dg.check();
     str.check();
     cfg.check();

//     bg.check();
//     produceFromPins();

//      p16.process();
//      p17.process();
//      blue.process();
//      gold.process();
      processControls();
      processTime();

  } else {
    // link not up, but continue to show indications on blue and gold
    blue.process();
    gold.process();
  }
}



