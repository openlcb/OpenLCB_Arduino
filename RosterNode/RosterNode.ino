/************************
RosterNode
A sketch to aid the development of the OpenLCB Traction and related Protocols
Copyright (C)2012 Railstars Limited

This file is part of RosterNode.

    RosterNode is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AdvancedDCS.  If not, see <http://www.gnu.org/licenses/>.
************************/

/***
RosterNode provides a middle layer between a proper OpenLCB throttle, and an OpenLCB-aware digital command station (DCS).
***/

#include <reset.h>
#include <can.h>
#include <OpenLCB.h>
#include "DCC_Proxy.h"

//TODO right now, as things are configured, each node uses 100 bytes of EEPROM, but this could change!
#define EEPROM_SIZE 100
#define EEPROM_OFFSET 10
#define NUM_SLOTS 1

DCC_Proxy train[NUM_SLOTS];
OLCB_NodeID train_nid[NUM_SLOTS];
OLCB_CAN_Link link;

void setup()
{
  Serial.begin(115200);
  Serial.println("RosterNode");
  Serial.println("Copyright (C) 2012 Railstars Limited");
  link.initialize();
  for(uint8_t i = 0; i < NUM_SLOTS; ++i)
  {
    train[i].initialize(); //DIY address space
    train[i].create(&link, &(train_nid[i]), (uint8_t *)(EEPROM_OFFSET + (i*EEPROM_SIZE))); //loads its own NID from EEPROM at address 0x10, no need to specify a specific NID
    link.addVNode(&(train[i]));
  }
  Serial.println("Ready!");
}

void loop()
{
  link.update();
}
