/**
 * Fast Clock Demo v1.1  (17k)
 *   A prototype of a FastClock OpenLCB board with eight Alarms
 * 
 *   D. P. Harris 2011
 *   from code by Bob Jacobsen 2010
 *      based on examples by Alex Shepherd and David Harris*
 *
 * Implements FastRates of 1:1 to 300:1
 * A FastClock Time-event is sent every second as [fast_clock_prefix hh mm]
 * A FC SetRate-event is sent evey 10 seconds as     [fast_clock_prefix FC|r rr]
 * A FC SetTime-event is [fast_clock_prefix hh+60 mm]
 *
 * Eight alarms, each can send a FC Alarm-event at its associated FC Time.  
 *
 * If a FC Time-event is received, it is ignored.  
 * If a FC SetTime event is received, the FC time is set to it.  
 * If a FC SetRate-event is received, the rate is set to the rate embedded in the event.  
 * If a FC Alarm-event is received, the clock is set to the time associated with that Alarm.  
 *
 * The User Interface (UI) uses the Blue / Gold buttons:
 *   A (Blue/Gold)+ sequence Sets or Nominates the Time, Rate or Alarms to learn a Teach-event.  
 *   Gold(Blue/Gold)+ sequence sends a Teach-event which includes the event associated
 *      with the current-Time, the Rate-, or any of the Alarms.  
 *
 * Rate is fixed rrrrrrrrrr.rr, 0.00, 0.25, 0.50, ..., 999.75
 *
 **/

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
#include "can.h"

//class foo{};

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
//#include "ui1.h"
#include "ButtonLed.h"
#include "LiquidCrystal.h"
#include "Alarm.h"

LiquidCrystal lcd(9, 8, 6, 5, 3, 2);
//LiquidCrystal lcd(12, 11, 6, 5, 3, 2);
//LiquidCrystal lcd(8, 9, 7, 6, 2, 4);  // PHI shield

OpenLcbCanBuffer     rxBuffer;	// CAN receive buffer
OpenLcbCanBuffer     txBuffer;	// CAN send buffer
//OpenLcbCanBuffer*    ptxCAN;

NodeID nodeid(1,13,0,0,0,18);    // This node's default ID
#define FC_prefix 0x1,0x1,0x99,0x1,0x1,0x1

LinkControl link(&txBuffer, &nodeid);

// Events this node can produce or consume, used by PCE and loaded from EEPROM by NM
#define nAlarms 8
#define nEvents nAlarms
Event events[nEvents] = { Event() };
int eventNum = nEvents;

// Standard Blue / Gold for UI
ButtonLed blue(17, LOW);
ButtonLed gold(19, LOW);

// Clock definitions
struct  time_s {
  uint8_t hour;
  uint8_t minute;
};
typedef struct time_s time_t;
time_t  cTime, zTime; 
long cPeriod;

// Set-up variables that are in the eeprom
struct e_t {
  time_t  alarms[nAlarms];
  int     cRate;
} e;
uint8_t alarmF[nAlarms];

// To receive an alarm-event
void pceCallback(int index) {
  // its an alarm
  cTime = e.alarms[index];         // if we receive an alarm-event, set the time
}

NodeMemory nm(0);  // allocate from start of EEPROM
void store() { nm.store(&nodeid, events, eventNum, (uint8_t*) &e, (uint16_t) sizeof(e)); }
PCE pce(events, eventNum, &txBuffer, &nodeid, pceCallback, store, &link);

#include "clock.h"
#include "clockDisplay.h"

void setup()
{
   delay(250);Serial.begin(112500);Serial.println("\nOlcClockGeneratorNode\n");
  //nm.forceInitAll(); // uncomment if need to go back to initial EEPROM state
  nm.setup(&nodeid, events, eventNum, (uint8_t*) &e, sizeof(e), 0);  
  // set event types, now that IDs have been loaded from configuration
  for (int i=0; i<eventNum; i++) {
      pce.newEvent(i,true,true); // produce, consume
  }
  // Initialize OpenLCB CAN connection
  OpenLcb_can_init();
  // Initialize OpenLCB CAN link controller
  link.reset();
  
  lcd.begin(16,2);
  //lcd.clear();
  //lcd.setCursor(0,0); 
  //lcd.print("hello");
  uiReset();
//  while(1) ;
 // uncomment next line to initialize alarms to 3000h, ie not set.  
 // zTime.hour=30;  zTime.minute=0;  for(int i=0; i<nAlarms; i++) e.alarms[i]=zTime;  store();
  cPeriod = 240000/e.cRate;
  cTime.hour=8;
  cTime.minute=0;
//  e.cRate = 240;
}

Alarm dispAlarm(100);
Alarm tick(1000);
Alarm FC_Alarm(1000);
Alarm FC_Rate(10000);

/*
uint8_t t=-1;
void loop() {
  if(uiCheck()) {
    uiReset();
  }
  if(tick.again(cPeriod)) {
    //Serial.println("tick");
    cTime.minute += 1;
    if(cTime.minute>59) {
      cTime.minute=0;
      cTime.hour += 1; 
      if(cTime.hour>23) {
        cTime.hour = 0;
      }
    }
  }
  if(dispAlarm.again(500)) {
    displayTime();
//    lcd.vDisplay();
  }
  // check to see if we are at or passed and alarm.  
  for(int i=0; i<nAlarms; i++)      // cycle through alarms, 
    if(cTime.hour==e.alarms[i].hour && cTime.minute==e.alarms[i].minute) {
      lcd.setCursor(0,1);
      lcd.print("Alarm sent"); 
      //sentEvent[i];  // if its time, send the matching event.  
    }
}
*/

NodeID FC_node = NodeID(FC_prefix);
Event FC_Event;

void sendFCEvent(int a, int b) {
    FC_Event = Event( FC_prefix, a, b);
    txBuffer.setProducerIdentified(&FC_Event);
    OpenLcb_can_queue_xmt_wait(&txBuffer);  // wait until buffer queued, but OK due to earlier check
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
        // To receive time and rate packets
        NodeID nid;
        Event ev;
        rxBuffer.getEventID(&ev);
        rxBuffer.getNodeID(&nid);
        if(FC_node.equals(&nid)) {
          if(ev.val[6]<24) {          // match time packets
            cTime.hour=ev.val[6]; 
            cTime.minute=ev.val[7];
          } 
          if(ev.val[6]>=0xFC) {        // match rate packets
            e.cRate= ((ev.val[6]&0x03)<<8) | ev.val[7]; 
            cPeriod=240000/e.cRate; 
            store();  
          }
        }
     }
     // periodic processing of any state changes
     pce.check();
  }
  
  // periodically update the FastClock
  if(tick.again(cPeriod)) {
    //Serial.println("tick");
    cTime.minute += 1;
    if(cTime.minute>59) {
      cTime.minute=0;
      cTime.hour += 1; 
      if(cTime.hour>23) {
        cTime.hour = 0;
      }
    }
    sendFCEvent(cTime.hour, cTime.minute);
  }

  // every second, send a FastClock time-event

  //if(FC_Alarm.again(1000)) { }

  // every 10 seconds, send a FastClock rate-event
  if(FC_Rate.again(10000)) { sendFCEvent(0xFC|(e.cRate>>8), e.cRate);  }

  // Check if an Alarm has past, if so send a FC Alarm-event
  for(int i=0; i<nAlarms; i++) {    // cycle through alarms, 
    if(cTime.hour==e.alarms[i].hour && cTime.minute==e.alarms[i].minute) {
      if(alarmF[i]) {
        pce.produce(i);  // if its time, send the matching event.  
        lcd.setCursor(0,1);
        lcd.print("Alarm sent"); 
        alarmF[i] = 0;
      }
    } else alarmF[i] = 1;
  }

  // process the User Interface
  if(uiCheck()) uiReset();
  
  // every 0.5 second, display the time.
  if(dispAlarm.again(500)) displayTime();
  
}


