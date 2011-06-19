//#include <MemoryFree.h>

#define DATAGRAM_LENGTH 5 //5 bytes at most. really.

#include <OLCB_AliasCache.h>
#include <OLCB_Buffer.h>
#include <OLCB_CAN_Buffer.h>
#include <OLCB_CAN_Link.h>
#include <OLCB_Datagram.h>
#include <OLCB_Datagram_Handler.h>
#include <OLCB_Event.h>
#include <OLCB_EventID.h>
#include <OLCB_Handler.h>
#include <OLCB_Link.h>
#include <OLCB_NodeID.h>
#include <OLCB_Stream.h>

#include <DCCPacket.h>
#include <DCCPacketQueue.h>
#include <DCCPacketScheduler.h>

#include <can.h>

#include "Locomotive.h"
#include "LocomotiveFactory.h"


DCCPacketScheduler packetScheduler;
LocomotiveFactory factory;

OLCB_NodeID nid(2,1,13,0,0,23);
OLCB_CAN_Link link(&nid);

void setup()
{
//  Serial.begin(115200);
//  Serial.println("Hello world!");
//  delay(50);
  packetScheduler.setup();
  link.initialize();
  factory.setLink((OLCB_Link*)&link);
//  Serial.println(freeMemory(),DEC);
//  Serial.println(sizeof(OLCB_Datagram), DEC);
}

void loop()
{
  packetScheduler.update();
  link.update();
//  Serial.println(freeMemory(),DEC);
}
