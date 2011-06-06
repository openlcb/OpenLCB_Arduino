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

Locomotive locos[25];

void setup()
{
  packetScheduler.setup();
}

void loop()
{
  packetScheduler.update();
}
