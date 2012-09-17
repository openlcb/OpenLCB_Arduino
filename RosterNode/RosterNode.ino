#include <ButtonLED.h>

// Permits control of a single locomotive with address 03.

#include <DCCPacket.h>
#include <DCCPacketQueue.h>
#include <DCCPacketScheduler.h>

#include <can.h>

#include <OpenLCB.h>

#include "OLCB_DCC_Train.h"

ButtonLed gold(GOLD);

class DCC_Train : public OLCB_Datagram_Handler, public OLCB_DCC_Train
{  
  public:
  
    void create(OLCB_Link *link, OLCB_NodeID *nid, DCCPacketScheduler *dcc)
    {
      OLCB_Datagram_Handler::create(link,nid);
      DCC_Train_create(dcc);
    }

    
  void update(void)
  {
    if(isPermitted())
    {
      OLCB_Datagram_Handler::update();
      DCC_Train_update();
    }
  }

  void datagramResult(bool accepted, uint16_t errorcode)
  {
     //Serial.print("The datagram was ");
     //if(!accepted)
       //Serial.print("not ");
     //Serial.println("accepted.");
     //if(!accepted)
     //{
       //Serial.print("   The reason: ");
       //Serial.println(errorcode,HEX);
     //}
  }
  
  void initialize(void)
  {
    DCC_Train_initialize();
  }
  
  uint16_t processDatagram(void)
  {
    Serial.println("Got a datagram");
    if(isPermitted()) // && (_rxDatagramBuffer->destination == *OLCB_Virtual_Node::NID))
    {
      Serial.println("   it's for us");
      return DCC_Train_processDatagram(_rxDatagramBuffer);
    }
  }
  
  
};

OLCB_NodeID nid(6,1,0,0,0,3);
DCC_Train myLoco;
OLCB_CAN_Link link;
DCCPacketScheduler controller;

void setup()
{
  Serial.begin(115200);
  
  Serial.println("SimpleDCS begin!");
  controller.setup();
  
  link.initialize();
  myLoco.initialize();
  myLoco.create(&link, &nid, &controller);
  link.addVNode(&myLoco);
  
  gold.on(0xFFFFFFFEL); // unready blink until intialized

  Serial.println("Initialization complete!");
}

void loop()
{
  controller.update();
  link.update();
  gold.process();
}
