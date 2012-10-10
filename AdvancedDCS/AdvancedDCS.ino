/************************
AdvancedDCS
A sketch to aid the development of the OpenLCB Traction and related Protocols
Copyright (C)2012 Railstars Limited

This file is part of AdvancedDCS.

    AdvancedDCS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AdvancedDCS.  If not, see <http://www.gnu.org/licenses/>.
    
******************

AdvancedDCS is designed to run on Railstars Io. A DCC signal is generated
on pins B.5 ad B.6. This signal must not be used to directly drive trains, but can be
used to drive a DCC booster, such as Railstars RAILbooster.

Software Requirements:
  "OpenLCB" library
  "CAN" library (from OpenLCB repository)
  "CmdrArduino" library (from github)
************/

#include <DCCHardware.h>
#include <DCCPacket.h>
#include <DCCPacketQueue.h>
#include <DCCPacketScheduler.h>
#include <can.h>
#include <OpenLCB.h>
#include "LocomotiveFactory.h"

DCCPacketScheduler dcs;
LocomotiveFactory factory;

OLCB_NodeID nid(2,1,13,0,0,1); //in the DIY range!
OLCB_CAN_Link link;


void setup()
{
  Serial.begin(115200);
  Serial.println("AdvancedDCS");
  Serial.println("Copyright (C) 2012 Railstars Limited");
  link.initialize();
  factory.initialize();
  factory.create(&link, &nid, &dcs);
  link.addVNode(&factory); //must be the first in line.
  dcs.setup();
  Serial.println("Ready!");
}

void loop()
{
  dcs.update();
  link.update();
}

