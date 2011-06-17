#include "Throttle.h"

#include "Throttle.h"


void Throttle::init(void)
{
  _speed = 0;
  _direction = true; //forwards
  for(int i = 0; i < NUM_FUNCS; ++i)
    _functions[i] = false; //off
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
    Serial.print("Attempting to attach to address ");
    Serial.println(_new_address,DEC);
    _dg.data[0] = DATAGRAM_MOTIVE;
    _dg.data[1] = DATAGRAM_MOTIVE_ATTACH; //attach
    _dg.length = 2;
    Serial.println("Making state ATTACHING in update()");
    _state = ATTACHING;
    sendDatagram(&_dg);
  }
  
  else if((_new_speed != _speed) || (_new_direction != _direction)) //if the speed has changed
  {
    _dg.data[0] = DATAGRAM_MOTIVE;
    _dg.data[1] = DATAGRAM_MOTIVE_SETSPEED; //set speed
    if(_new_direction) //forward
      _dg.data[2] = 1;
    else
      _dg.data[2] = 2;
    _dg.data[3] = _new_speed; //in percent throttle
    _dg.length = 4;
    _state = SETTING_SPEED;
    Serial.print("Setting speed to ");
    Serial.println(_new_speed, DEC);
    Serial.print(" and direction to ");
    Serial.println(_dg.data[2], DEC);
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
   Serial.print("The datagram was ");
   if(!accepted)
     Serial.print("not ");
   Serial.println("accepted.");
   if(!accepted)
   {
     Serial.print("   The reason: ");
     Serial.println(errorcode,HEX);
  }
  Serial.print("And the state was:");
  Serial.println(_state, DEC);
  
  if(_state == SETTING_SPEED)
  {
    if(accepted)
    {
      _speed = _new_speed;
      _direction = _new_direction;
      Serial.print("Speed is now set to ");
      Serial.println(_new_speed, DEC);
      Serial.print(" and direction to ");
      if(_new_speed)
        Serial.println('1');
      else
        Serial.println('2');
    }
    else
    {
      _new_speed = _speed; //reset new_speed request
    }
    _state = IDLE;
  }
  
  else if(_state == SETTING_FUNCTION)
  {
    if(accepted)
    {
      _functions[_new_function_ID] = _new_function_val;
    }
    _state = IDLE;
  }
  
  else if (_state == ATTACHING)
  {
    if(!accepted)
    {
      _state = IDLE; //force another go. TODO make this more nuanced, so it will give up!
    }
  }
}

bool Throttle::processDatagram(void)
{
  if((_rxDatagramBuffer->data[0] == DATAGRAM_MOTIVE) && (_rxDatagramBuffer->data[1] == DATAGRAM_MOTIVE_ATTACHED))
  {
//    if(_new_address == (_rxDatagramBuffer->source.val[4] << 8) & (_rxDatagramBuffer->source.val[4]))
//    {
      _state = IDLE;
      _attached = true;
      _address = _new_address; //cheap hack. Will cause problems if setAddress is called rapidly several times in a row! we should look at the source
      return true;
//    }
  }
  Serial.println("NAKing datagram");
  Serial.print("len ");
  Serial.println(_rxDatagramBuffer->length, DEC);
  for(int i = 0; i < _rxDatagramBuffer->length; ++i)
    Serial.println(_rxDatagramBuffer->data[i], HEX);
  return false;
}

void Throttle::setSpeed(unsigned short speed, boolean forward)
{
  _new_speed = speed;
  _new_direction = forward;
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
    Serial.print("Releasing address ");
    Serial.println(_address,DEC);
    _dg.data[0] = DATAGRAM_MOTIVE;
    _dg.data[1] = DATAGRAM_MOTIVE_RELEASE; //set function
    _dg.length = 2;
    sendDatagram(&_dg);
  }

  _new_address = address;
  _address = 0;
  if(address)
  {
    //now, set new address
    _state = IDLE; //not really, but this will do.
    _attached = false;
    _dg.destination.set(6,1,0,0,(_new_address&0xFF00) >> 8,(_new_address&0x00FF));
  }
}
