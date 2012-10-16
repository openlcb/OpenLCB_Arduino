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

 ***************************************************************************************/

#include "LocomotiveFactory.h"
#include "OLCB_CAN_Link.h"
#include <string.h>

#define COMMAND_STATION_EVENT 01,01,00,00,00,00,04,01

void
LocomotiveFactory::initialize(void)
{
  _station_number = 1;
  _producer_identified_flag = true;
}

void
LocomotiveFactory::create(OLCB_Link *link, OLCB_NodeID *nid,
    DCCPacketScheduler *dcs)
{
  _dcs = dcs;
  OLCB_Virtual_Node::create(link, nid);
}

bool
LocomotiveFactory::handleMessage(OLCB_Buffer *buffer)
{
  Serial.println("got message");
  if (buffer->isVerifyNIDGlobal())
    {
      Serial.println("Got VerifyNIDGlobal!");

      //we only get here if the message is a VerifyNIDGlobal
      if (buffer->length == 6) //it has a nodeid
        {
          OLCB_NodeID nid;
          buffer->getNodeID(&nid);

          //check the NID
          if ((nid.val[0] == 6) && (nid.val[1] == 1) && (nid.val[2] == 0)
              && (nid.val[3] == _station_number)) //it's for a DCC train
            {
              //find a slot for it
              // Serial.println("LocoFactory: Got a request to create a new loco vnode");
              for (uint8_t i = 0; i < NUM_SLOTS; ++i)
                {
                  Serial.println(_trains[i].getAddress());
                  if (!_trains[i].getAddress()) //an empty slot is found!
                    {
                      Serial.print("    Installing in slot ");
                      Serial.println(i, DEC);
                      //  Serial.println(freeMemory(),DEC);
                      Serial.println("copying NID");
                      buffer->getNodeID(&(_train_nids[i])); //copy over.
                      _train_nids[i].print();
                      Serial.println("creating train");
                      _trains[i].create(_link, &(_train_nids[i]), _dcs);
                      Serial.println("register vnode");
                      _link->addVNode(&(_trains[i])); //how can we undo this later?
                      // Serial.println(freeMemory(),DEC);
                      _trains[i].verified = false; //just in case
                      //  Serial.println(freeMemory(),DEC);
                      Serial.println("Done installing loco");
                      return true; //Notice that we're actually not yet ready to send out the verifiedNID packet, as we don't yet have an alias.
                      //That's up to the virtual node to do on its own!
                    }
                }
              Serial.println("    Out of slots. Too bad."); //TODO Need to figure out what to do in this case?
              //It won't do to let the throttle requesting the loco just hang, we need to tell it something informative.
              //Problem is, without enough memory, we can't request an alias to go with the requested NID, and so
              //we can't respond /as/ the loco being queried. Need a mechanism for telling a throttle about this?
              //We could use the LocoFactory's alias, then pass it a datagram to inform it of why it's request
              //is invalid, and then send out a message indicating that the alias is being invalidated for that NID. Need
              //a method for invalidating aliases.
            }
        }
//  //  Serial.println("No room!");
//  //  Serial.println(freeMemory(),DEC);
}
      else if (buffer->isIdentifyProducers())
        {
          //get the EventID, see if it matches the preset one.
          OLCB_Event evt, my_evt(COMMAND_STATION_EVENT);
          buffer->getEventID(&evt);
          bool match = true;
          for (uint8_t i = 0; i < 8; ++i)
            {
              if (evt.val[i] != my_evt.val[i])
                match = false;
            }
          if (match) //send an event produced
            {
              _producer_identified_flag = true;
              return true;
            }
        }
      return false; //no room availble, or not a request for a loco. (see above)
    }

  void
  LocomotiveFactory::update(void)
  {
    if (!isPermitted())
      return;

    //check to see if we need to emit a producer identified event
    if (_producer_identified_flag)
      {
        OLCB_Event e(COMMAND_STATION_EVENT);
        while (!_link->sendProducerIdentified(NID, &e))
          ;
        _producer_identified_flag = false;
      }

    //This is where we should force a verifiedID from any recently created loco, as it would have missed
    // the initial request.
    for (uint8_t i = 0; i < NUM_SLOTS; ++i)
      {
        if (!_trains[i].verified && _trains[i].NID->alias) //"verified" is just a flag to let us know that it hasn't verified its
        // NID yet; checking alias ensures that it has been assigned one. In this case, we need to tell the node to
        // send out a verified ID message.
          {
            //  Serial.println("Sending VerifedNID");
            //((OLCB_CAN_Link*) _link)->sendVerifiedNID(_trains[i].NID);
            //_trains[i].verified = true;
          }
      }
  }

