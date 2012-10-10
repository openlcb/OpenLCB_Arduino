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

#include "Train.h"

void Train::initialize(void)
{
  _refresh_timer = _release_timer = millis();
  _address = 0;
  _address_kind = 0;
  _speed = 0;
  _FX = 0;
  verified = false;
}

void Train::create(OLCB_Link *link, OLCB_NodeID *nid, DCCPacketScheduler *dcs)
{
  _dcs = dcs;
  //decode an RP9.2.1 address
  //TODO ACCESSORY DECODERS
  if(nid->val[4]) //a long address
  {
    _address = ( (nid->val[4] & 0x3F) << 8) | (nid->val[5]);
    _address_kind = 1;
  }
  else
  {
    _address = nid->val[5];
    _address_kind = 0;
  }
  //TODO figure out address type!
  OLCB_Datagram_Handler::create(link,nid);
}

    
void Train::update(void)
{
  if(!isPermitted())
    return;

  //TODO send verified node id if we can!
  if(!verified)
  {
    Serial.println("train sending Verified NID");
    _link->sendVerifiedNID(NID);
    verified = true;
  }

  //if we haven't gotten a command in a while, time out and release ourselves
  if(_address)
  {
    uint32_t new_time = millis();
    if( (new_time - _release_timer) > RELEASE_TIMEOUT)
    {
      //release ourselves!
      Serial.print("Train no ");
      Serial.print(_address);
      Serial.println(" self-releasing from timeout");
      _link->removeVNode(this);
      initialize();
      return;
    }
    
    if( (new_time - _refresh_timer) > REFRESH_TIMEOUT)
    {
      _refresh_timer = new_time;
      //send some commdands to the DCS!
      updateSpeed();
      _dcs->setFunctions(_address, _address_kind, _FX);
      //TODO send function update commands too
    }
  }
  
  OLCB_Datagram_Handler::update();
}

void Train::updateSpeed(void)
{
  switch(_speed_steps)
  {
    case 14:
      _dcs->setSpeed14(_address, _address_kind, _speed);
      break;
    case 28:
      _dcs->setSpeed28(_address, _address_kind, _speed);
      break;
    case 128:
      _dcs->setSpeed128(_address, _address_kind, _speed);
      break;
  }
}

void Train::datagramResult(bool accepted, uint16_t errorcode)
{
   
}
  
uint32_t Train::getAddress(uint8_t* data)
{
  uint32_t val = 0;
  val |= ((uint32_t)data[2]<<24);
  val |= ((uint32_t)data[3]<<16);
  val |= ((uint32_t)data[4]<<8);
  val |= ((uint32_t)data[5]);
  return val;
}

uint8_t Train::decodeSpace(uint8_t* data)
{
  int val;
  switch (data[1]&0x03)
  {
    case 0x03:
      val = 0xFF;
      break;
    case 0x01:
      val = 0xFD;
      break;
    case 0x02:
      val = 0xFE;
      break;
    case 0x00:
      val = data[6];
      break;
  }
  return val;
}

uint8_t Train::decodeLength(uint8_t* data)
{
  return data[6];
}

uint16_t Train::MACProcessWrite(void)
{
    Serial.println("in Process Write");
    uint32_t address = getAddress(_rxDatagramBuffer->data);
    uint8_t space = decodeSpace(_rxDatagramBuffer->data);
    uint8_t datagram_offset = (_rxDatagramBuffer->data[1]&0x03)?6:7; //if the space is encoded in the flags in data[1], then the payload begins at byte 6; else, the payload begins after the space byte, at byte 7.
    uint8_t length = _rxDatagramBuffer->data[datagram_offset];
    
    Serial.println(space, HEX);
    Serial.print("address: ");
    Serial.println(address, HEX);
    
    if(space != 0xFE) //all memory space is what we use here
      return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;
     
     
    Serial.println(address, HEX);
    switch(address)
    {
      case 0x00: //speed
        _speed = (int8_t)(_rxDatagramBuffer->data[datagram_offset]);
        _speed_steps = _rxDatagramBuffer->data[datagram_offset+1];
        updateSpeed();
        _release_timer = millis(); //reset the auto-release timer.
        Serial.print("updating speed: ");
        Serial.println(_speed, DEC);
        return DATAGRAM_ERROR_OK;
      case 0x01: //FX
        _FX = (_rxDatagramBuffer->data[datagram_offset] << 8) | _rxDatagramBuffer->data[datagram_offset+1];
        _dcs->setFunctions(_address, _address_kind, _FX);
        _release_timer = millis(); //reset the auto-release timer.
        Serial.print("updating FX: ");
        Serial.println(_FX, DEC);
        return DATAGRAM_ERROR_OK;
    }
    
    return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;
}
  
uint16_t Train::processDatagram(void)
{
  if(!isPermitted())
    return DATAGRAM_REJECTED;
    
  Serial.println("got a datagram");
  if( (_rxDatagramBuffer->length) && (_rxDatagramBuffer->data[0] == MAC_PROTOCOL_ID) ) //we have a memory configuration datagram, which is all we care about right now.
  {
    Serial.println("got a MAC datagram");
    
    switch(_rxDatagramBuffer->data[1]&0xC0)
    {
      case MAC_CMD_READ:
        break;
      case MAC_CMD_WRITE:
        return MACProcessWrite();
      case MAC_CMD_OPERATION:
        //TODO
        break;
    }
  }
  return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;
}

