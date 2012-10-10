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
************************/

#ifndef __LOCOMOTIVE_FACTORY_H__
#define __LOCOMOTIVE_FACTORY_H__  

#include <OpenLCB.h>
#include <DCCPacketScheduler.h>
#include "Train.h"


#define NUM_SLOTS 5

class LocomotiveFactory : public OLCB_Virtual_Node
{
  public:
  
    void create(OLCB_Link *link, OLCB_NodeID *nid, DCCPacketScheduler *dcc);
    void initialize(void);
    void update(void);
    bool handleMessage(OLCB_Buffer *buffer);
    
  private:
    DCCPacketScheduler *_dcs;
     Train _trains[NUM_SLOTS];
     OLCB_NodeID _train_nids[NUM_SLOTS];
     uint8_t _station_number;
     uint8_t _producer_identified_flag;
};

#endif  
