#include <ButtonLED.h>

#include <EEPROM.h>

#include <OpenLCB.h>

#include <can.h>

#include "MyEventHandler.h"
#include "MyConfigHandler.h"
#include "MyBlueGoldHandler.h"
#include "MyInfoHandler.h"

uint32_t old_time;

//==============================================================
// Io_16P_16C_default
// This is the default sketch for Railstars Io NMRAnet demonstration boards
// Sets up 8 inputs (and hence 16 producers) on the "input" header, and 8 outputs (and hence 16 consumers) on the "output" header
// As well as a blue-gold interface for programming both
//
// © 2012 D.E. Goodman
// License TBA
//==============================================================

OLCB_NodeID nodeid;

//Allocate memory for the event pool
OLCB_Event event_pool[32];

/* define the CAN link to the outside world */
OLCB_CAN_Link link;

/* declare the PC Event Report handler */
MyEventHandler pce;
MyConfigHandler cfg;
MyBlueGoldHandler bg;
MyInfoHandler info;

/* define the Blue/Gold devices */
ButtonLed blue(BLUE, LOW); // button on pin 14
ButtonLed gold(GOLD, LOW); // button on pin 15

int available()
{
  byte stack = 1;
  extern char *__brkval;
  extern char *__malloc_heap_end;
  extern size_t __malloc_margin;
  if (__malloc_heap_end)
    return __malloc_heap_end - __brkval;
  return (byte*)((byte*)(&stack) - (byte*)__brkval) - (byte*)__malloc_margin;
} 

void loadNodeID(OLCB_NodeID *nid)
{
  //The NodeID is stored at the very end of EEPROM
  //on the AT90CAN128, the last EEPROM address is 0x0FFF
  //So:
  for(uint16_t i = 0; i < 6; ++i)
    nid->val[i] = EEPROM.read(0x0FFA+i);
  if((nid->val[0] == 0xFF) &&  (nid->val[1] == 0xFF) && (nid->val[2] == 0xFF) && (nid->val[3] == 0xFF) && (nid->val[4] == 0xFF) && (nid->val[5] == 0xFF) )
  {
    //Serial.println("NO NODE ID!");
    while(1);
  }
}

// =============== Setup ===================

void setup()
{
  //Serial.begin(115200); Serial.println("Io 16C 16P default");
  //first, set up inputs and outputs, setting pull-up resistors on inputs
  for(int i = 0; i < 8; ++i) //outputs
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  for(int i = 8; i < 16; ++i) //inputs
  {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }

  //now, load the NodeID from EEPROM
  loadNodeID(&nodeid);

  //nodeid.print();
  link.initialize();
  pce.create(&link, &nodeid);
  pce.initialize(event_pool, 32); //set up a space for 32 events: 16 producers and 16 consumers TODO REMOVE THIS!?
  cfg.create(&link, &nodeid, &pce);
  bg.create(&link, &nodeid, &pce);
  info.create(&link, &nodeid);
  link.addVNode((OLCB_Virtual_Node*)&pce);
  link.addVNode((OLCB_Virtual_Node*)&cfg);
  link.addVNode((OLCB_Virtual_Node*)&bg);
  link.addVNode((OLCB_Virtual_Node*)&info);
}

// ================ Loop ===================

void loop()
{
  // OpenLCB statndard processing:
  link.update();
  if (link.wasActiveSet()) {
    link.resetWasActive();
    blue.blink(0x01);
  }
}

// ---------------------------------------------------


// TODO
// testConfigurationProtocol
// simpleNodeIdentificationInformation
// ProtocolIdentificationProtocol
// unknownMtiAddressed

