#include "Locomotive.h"

void Locomotive::reset(void)
{
  available = true;
  state = LOCOMOTIVE_INITIAL;
  sendAttached = false;
  verified = false;
  speed = 0;
  direction = 1;
  packetScheduler.setSpeed128(getDCCAddress(), 1);
  ((OLCB_CAN_Link*)_link)->sendAMR(NID);
  NID->set(0,0,0,0,0,0); //reset our NID, as it is no longer valid. We assume that our NID was malloc'd by setNID, and is not pointing at the links's NID! A decent assumption, but needs to be stated.
  throttle.set(0,0,0,0,0,0);
  _link->removeHandler(this);
}

void Locomotive::update(void)
{
  //handle periodic sending of DCC packet for keep-alive
  uint16_t cur_time = millis();
  if((cur_time - timer) > 5000) //5 seconds have passed, time to update!
  {
    timer = cur_time;
    packetScheduler.setSpeed128(getDCCAddress(), speed*direction);
  }
  
  OLCB_Datagram_Handler::update();
}

bool Locomotive::processDatagram(void)
{
//  Serial.println("Locomotive Got a datagram!");
  if(_rxDatagramBuffer->data[0] == DATAGRAM_MOTIVE) //is this a datagram for loco control?
  {
//    Serial.println("Got a motive datagram!");
    switch(_rxDatagramBuffer->data[1])
    {
    case DATAGRAM_MOTIVE_ATTACH:
//      Serial.println("Request attach!");
      return attachDatagram();
    case DATAGRAM_MOTIVE_RELEASE:
//      Serial.println("Request release!");
      return releaseDatagram();
    case DATAGRAM_MOTIVE_SETSPEED:
  //    Serial.println("Request set speed!");
      return setSpeedDatagram();
    case DATAGRAM_MOTIVE_SETFUNCTION:
  //    Serial.println("Request set function!");
      return setFunctionDatagram();
    }
  }
//  Serial.println("Datagram not handleable by me");
  return false;
}

void Locomotive::datagramResult(bool accepted, uint16_t errorcode)
{
  // cases to handle:
  //  *attached NAKd (make self available)
  if(state == LOCOMOTIVE_ATTACHING)
  {
    if(accepted)
    {
  //    Serial.println("Attached request ack'd");
      state = LOCOMOTIVE_INITIAL;
      available = false;
    }
    else //the ATTACHED datagram was NAK'd? ReallY? reset.
    {
      reset();
  //    Serial.println("Attached request NOT ack'd, resetting");
    }
  }
  else if(state == LOCOMOTIVE_RELEASING)
  {
    if(accepted)
    {
       //release!
//       Serial.println("Released ack'd");
       reset(); //change nothing else!
    }
    else
    {
//       Serial.println("Released not ack'd!?"); //not even sure what to do in this case. Release anyway?!
       reset();
    }
  }
}

bool Locomotive::attachDatagram(void)
{
  OLCB_Datagram d;
  d.destination.copy(&(_rxDatagramBuffer->source));
  d.length = 2;
  d.data[0] = DATAGRAM_MOTIVE;
  //check the source
  if(available || _rxDatagramBuffer->source == throttle) //if available, or if coming from an already-attached throttle (perhaps it rebooted?)
  {
//    Serial.println("Preparing to attach...");
    throttle.copy(&(_rxDatagramBuffer->source)); //really shouldn't do this until the attached datagram is ACKd
    d.data[1] = DATAGRAM_MOTIVE_ATTACHED;
    state = LOCOMOTIVE_ATTACHING; //have to catch the ACK to complete the attachement.
  }
  else //not free; send AttachDenied datagram in response.
  {
    d.data[1] = DATAGRAM_MOTIVE_ATTACH_DENIED;
    //don't worry about catching the ACK or NAK.
  }
  sendDatagram(&d);
  return true;
}

bool Locomotive::releaseDatagram(void)
{
  if(!available && _rxDatagramBuffer->source == throttle)
  {
//    Serial.println("  sending released datagram!");
    OLCB_Datagram d;
    d.destination.copy(&(_rxDatagramBuffer->source));
    d.length = 2;
    d.data[0] = DATAGRAM_MOTIVE;
    d.data[1] = DATAGRAM_MOTIVE_RELEASED;
    sendDatagram(&d); //THIS IS BAD FORM releasing before making sure release is ACK'd. Does it matter, though?
    state = LOCOMOTIVE_RELEASING;
    return true;
  }
  //Serial.println("  not attached!");
  return false; //what you talkin' 'bout, Willis?
}

bool Locomotive::setSpeedDatagram(void)
{
  //TODO CHECK DATAGRAM LENGTH!!
  if(!available && _rxDatagramBuffer->source == throttle)
  {
//    Serial.println("  setting speed!");
    //Speed in data[3], and direction in data[2].
    speed = map(_rxDatagramBuffer->data[3], 0, 100, 1, 127);
    if(_rxDatagramBuffer->data[2] == 1)
      direction = 1;
    else
      direction = -1;
    packetScheduler.setSpeed128(getDCCAddress(), speed*direction);
    return true;
  }
//  Serial.println("  not attached!");
  return false;
}

bool Locomotive::setFunctionDatagram(void)
{
  //TODO CHECK DATAGRAM LENGTH!!
  if(!available && _rxDatagramBuffer->source == throttle)
  {
//    Serial.print("Setting function ");
//    Serial.println(_rxDatagramBuffer->data[2]-1,DEC);
//    Serial.print("to ");
//    Serial.println(_rxDatagramBuffer->data[3],DEC);
//    Serial.print("  setting functions to ");
    //function no. in data[2] and on/off in data[3]
    if(_rxDatagramBuffer->data[3]) //function on
      functions |= (1<<(_rxDatagramBuffer->data[2]-1));
    else //function off
    functions &= ~(1<<(_rxDatagramBuffer->data[2]-1));
//    Serial.println(functions, BIN);
    packetScheduler.setFunctions(getDCCAddress(), functions);
    return true;
  }
  //Serial.println("  not attached!");
  return false;
}

uint16_t Locomotive::getDCCAddress(void)
{
  return ((uint16_t)(NID->val[4])<<16) | (NID->val[5]);
}

