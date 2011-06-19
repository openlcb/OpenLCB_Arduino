#include "Throttle.h"

#include "Throttle.h"


void Throttle::init(void)
{
  _speed = 0;
  _direction = FORWARD;
  for(int i = 0; i < NUM_FUNCS; ++i)
    _functions[i] = !i; //start with only headlights on
  _address = 0; //no address
  _attached = false;
  _set_function = false;
}

void Throttle::update(void)
{
  //If address != 0 and attached = false, that means we are attempting to attach
  //to a loco. Give it another go.
  if(_new_address && !_attached && (_state==IDLE))
  {
//    Serial.print("Attempting to attach to address ");
//    Serial.println(_new_address,DEC);
    _dg.data[0] = DATAGRAM_MOTIVE;
    _dg.data[1] = DATAGRAM_MOTIVE_ATTACH; //attach
    _dg.length = 2;
//    Serial.println("Making state ATTACHING in update()");
    _state = ATTACHING;
    sendDatagram(&_dg);
  }
  
  else if((_new_speed != _speed) || (_new_direction != _direction)) //if the speed has changed
  {
    _dg.data[0] = DATAGRAM_MOTIVE;
    _dg.data[1] = DATAGRAM_MOTIVE_SETSPEED; //set speed
    if(_new_direction == FORWARD) //forward
      _dg.data[2] = OLCB_FORWARD;
    else
      _dg.data[2] = OLCB_REVERSE;
    _dg.data[3] = _new_speed; //in percent throttle
    _dg.length = 4;
    _state = SETTING_SPEED;
    sendDatagram(&_dg);
  }
  
  else if(_set_function) //if a function has changed
  {
    _dg.data[0] = DATAGRAM_MOTIVE;
    _dg.data[1] = DATAGRAM_MOTIVE_SETFUNCTION; //set function
    _dg.data[2] = _new_function_ID+1;
    _dg.data[3] = _new_function_val; //in percent throttle
    _dg.length = 4;
    _state = SETTING_FUNCTION;
    _set_function = false;
    sendDatagram(&_dg);
  }
  
  OLCB_Datagram_Handler::update();
}


void Throttle::datagramResult(bool accepted, uint16_t errorcode)
{
/*   Serial.print("The datagram was ");
   if(!accepted)
     Serial.print("not ");
   Serial.println("accepted.");
   if(!accepted)
   {
     Serial.print("   The reason: ");
     Serial.println(errorcode,HEX);
  }
  Serial.print("And the state was:");
  switch(_state)
  {
      case SETTING_SPEED:
        Serial.println("SETTING_SPEED");
        break;
      case SETTING_FUNCTION:
        Serial.println("SETTING_FUNCTION");
         break;
      case ATTACHING:
        Serial.println("ATTACHING");
         break;
      case RELEASING:
        Serial.println("RELEASING (should never happen!?)");
        break;
       case IDLE:
         Serial.println("IDLE");
         break;
  }*/
    
  if(_state == SETTING_SPEED)
  {
    if(accepted)
    {
      _speed = _new_speed;
      _direction = _new_direction;
//      Serial.print("Speed is now set to ");
//      Serial.println(_new_speed, DEC);
//      Serial.print(" and direction to ");
//      if(_new_direction == FORWARD)
//        Serial.println("FORWARD");
//      else
//        Serial.println("REVERSE");
    }
    else
    {
      _new_speed = _speed; //reset new_speed request
      _new_direction = _direction;
    }
    _state = IDLE;
  }
  
  else if(_state == SETTING_FUNCTION)
  {
    if(accepted)
    {
      _functions[_new_function_ID] = _new_function_val;
//      Serial.print("Function ");
//      Serial.print(_new_function_ID, DEC);
//      Serial.print(" is now set to ");
//      Serial.print(_new_function_val, DEC);
    }
    _state = IDLE;
  }
  
  else if (_state == ATTACHING)
  {
    if(!accepted)
    {
      _state = IDLE; //force another go. TODO make this more nuanced, so it will give up!
      _speed = _new_speed = 0;
      _direction = _new_direction = FORWARD;
      for(int i = 0; i < NUM_FUNCS; ++i)
        _functions[i] = !i; //start with only headlights on
    }
  }
}

bool Throttle::processDatagram(void)
{
  if((_rxDatagramBuffer->data[0] == DATAGRAM_MOTIVE) && (_rxDatagramBuffer->data[1] == DATAGRAM_MOTIVE_ATTACHED))
  {
//    Serial.println("Received Datagram ATTACHED");
//    Serial.print(" from ");
//    Serial.println(_rxDatagramBuffer->source.alias, DEC);
//    Serial.println((_rxDatagramBuffer->source.val[4] << 8) & (_rxDatagramBuffer->source.val[4]), DEC);
//    if(_new_address == (_rxDatagramBuffer->source.val[4] << 8) & (_rxDatagramBuffer->source.val[4])) //TODO WIll this cause problems?
//    {
      _state = IDLE;
      _attached = true;
      _address = _new_address;
//      Serial.println("ACKing");
      return true;
//    }
  }
  else if((_rxDatagramBuffer->data[0] == DATAGRAM_MOTIVE) && (_rxDatagramBuffer->data[1] == DATAGRAM_MOTIVE_RELEASED) && (_state == RELEASING))
  {
//    Serial.println("Recevied Datagram RELEASED");
    _state = IDLE;
    return true; //whatever, just ACK it.
  }

//  Serial.println("NAKing datagram:");
//  Serial.print("len ");
//  Serial.println(_rxDatagramBuffer->length, DEC);
//  for(int i = 0; i < _rxDatagramBuffer->length; ++i)
//    Serial.println(_rxDatagramBuffer->data[i], HEX);
//  Serial.println("NAKing datagram");
  return false;
}

void Throttle::setSpeed(unsigned short speed, boolean direction)
{
  _new_speed = speed;
  _new_direction = direction;
}

void Throttle::setFunction(byte funcID, boolean on)
{
  _new_function_ID = funcID;
  _new_function_val = on;
  _set_function = true;
}

void Throttle::setAddress(unsigned int address)
{
  //first, if address!=0, release existing locomotive
  if(_address)
  {
//    Serial.print("Releasing address ");
//    Serial.println(_address,DEC);
    _dg.data[0] = DATAGRAM_MOTIVE;
    _dg.data[1] = DATAGRAM_MOTIVE_RELEASE; //set function
    _dg.length = 2;
    _state = RELEASING; //This is to force the throttle to wait for a "Released" datagram;
    sendDatagram(&_dg);
  }
  else
  {
    _state = IDLE;
  }
//  Serial.print("setAddress to: ");
//  Serial.print(address, DEC);
  _new_address = address;
  _address = 0; //not yet!
    //now, set new address
//    _state = IDLE; //not really, but this will do.
  _attached = false;
  _dg.destination.set(6,1,0,0,(_new_address&0xFF00) >> 8,(_new_address&0x00FF)); //set the locomotive as the destination address for future comms...
}
