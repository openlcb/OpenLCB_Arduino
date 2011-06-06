#include "Locomotive.h"

void Locomotive::init(void)
{
  available = true;
  state = LOCOMOTIVE_INITIAL;
}

void Locomotive::update(void)
{
  //handle periodic sending of DCC packet for keep-alive TOD
  uint16_t cur_time = millis();
  if((cur_time - timer) > 5000) //5 seconds have passed, time to update!
  {
    timer = cur_time;
    packetScheduler.setSpeed128(getDCCAddress(), speed*direction);
  }
}

bool Locomotive::processDatagram(void)
{
  if(_rxDatagramBuffer->data[0] == DATAGRAM_MOTIVE) //is this a datagram for loco control?
  {
    switch(_rxDatagramBuffer->data[1])
    {
    case DATAGRAM_MOTIVE_ATTACH:
      return attachDatagram();
    case DATAGRAM_MOTIVE_RELEASE:
      return releaseDatagram();
    case DATAGRAM_MOTIVE_SETSPEED:
      return setSpeedDatagram();
    case DATAGRAM_MOTIVE_SETFUNCTION:
      return setFunctionDatagram();
    }
  }
  return false;
}

void Locomotive::datagramResult(bool accepted, uint16_t errorcode)
{
  // cases to handle:
  //  *attached NAKd (make self available) TODO
  if((state == LOCOMOTIVE_ATTACHING) && accepted)
  {
    state = LOCOMOTIVE_ATTACHED;
  }
  else
  {
    state = LOCOMOTIVE_INITIAL;
    available = true;
  }
}

bool Locomotive::attachDatagram(void)
{
  OLCB_Datagram d;
  d.destination.copy(&(_rxDatagramBuffer->source));
  d.length = 1;
  if(available) //if free!
  {
    throttle.copy(&(_rxDatagramBuffer->source));
    available = false;
    d.data[0] = DATAGRAM_MOTIVE_ATTACHED;
    state = LOCOMOTIVE_ATTACHING;
  }
  else //not free!
  {
    d.data[0] = DATAGRAM_MOTIVE_ATTACH_DENIED;
  }
  sendDatagram(&d);
  return true;
}

bool Locomotive::releaseDatagram(void)
{
  if(!available && _rxDatagramBuffer->source == throttle)
  {
    OLCB_Datagram d;
    d.destination.copy(&(_rxDatagramBuffer->source));
    d.length = 1;
    d.data[0] = DATAGRAM_MOTIVE_RELEASED;
    sendDatagram(&d); //THIS IS BAD FORM releasing before making sure release is ACK'd. TODO
    available = false;
    state = LOCOMOTIVE_INITIAL;
    return true;
  }
  return false; //what you talkin' 'bout, Willis?
}

bool Locomotive::setSpeedDatagram(void)
{
  if(!available && _rxDatagramBuffer->source == throttle)
  {
    //Speed in data[2], and direction in data[1].
    speed = (_rxDatagramBuffer->data[2] / 100.0) * 127;
    if(_rxDatagramBuffer->data[1] == 1)
      direction = 1;
    else
      direction = -1;
    packetScheduler.setSpeed128(getDCCAddress(), speed*direction);
    return true;
  }
  return false;
}

bool Locomotive::setFunctionDatagram(void)
{
  if(!available && _rxDatagramBuffer->source == throttle)
  {
    //function no. in data[1] and on/off in data[2]
    if(_rxDatagramBuffer->data[2]) //function on
      functions |= (1<<_rxDatagramBuffer->data[1]);
    else //function off
    functions &= ~(1<<_rxDatagramBuffer->data[1]);
    packetScheduler.setFunctions(getDCCAddress(), (functions>>16), (functions&0xFF), 0);
    return true;
  }
  return false;
}

uint16_t Locomotive::getDCCAddress(void)
{
  return ((uint16_t)(NID->val[4])<<16) | (NID->val[5]);
}

