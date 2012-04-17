#include "MyBlueGoldHandler.h"

#define ShortBlinkOn   0x00010001L
#define ShortBlinkOff  0xFFFEFFFEL

#define PRESS_DELAY  75
#define LONG_DOUBLE  3000
#define VERY_LONG_DOUBLE  8000

void MyBlueGoldHandler::create(OLCB_Link *link, OLCB_NodeID *nid, MyEventHandler *eventHandler)
{
  OLCB_Virtual_Node::create(link,nid);
  _index = -1;
  _input_index = -1;
  _input_count = 0;
  _started = false;
  _event_handler = eventHandler;

  // initial blue/gold setting
  blue.on(0); // turn off 
  blue.process();
  _last_blue = blue.state;
  gold.on(UNREADY_BLINK); // unready blink until intialized
  gold.process();
  _last_gold = gold.state;
  //get the inputs to monitor
  _input_buttons = _event_handler->getInputs();
  _double_press = _last_double = 0;
}

void MyBlueGoldHandler::moveToIdle(bool reset)
{
  blue.on(0x00000000);
  gold.on(READY_BLINK);
  if(reset)
  {
    for (uint8_t i = 0; i < 32; i++)
    {
      _event_handler->markToTeach(i, false);
      _event_handler->markToLearn(i, false);
    }
  }
  for(uint8_t i = 0; i < 8; ++i)
  {
    digitalWrite(i, LOW);
  }
  _index = -1;
  _input_index = -1;
  _event_handler->disInhibit();
  _state = BG_IDLE;
}

void MyBlueGoldHandler::update(void)
{
  if(!isPermitted())
  {
    return;
  }


  if (!_started)
  {
    _started = true;
    gold.on(READY_BLINK); // turn off waiting to init flash, start heartbeat ready blink
  }

  //determine which buttons have been pressed since last time.

  blue.process();
  gold.process();
  for(uint8_t i = 0; i < 8; ++i)
    // first, check and see if we've got a double-press
    _double_state = (blue.state && gold.state);
  if(_double_state)
  {
    _last_blue = blue.state;
    _last_gold = gold.state;
    if(blue.duration > VERY_LONG_DOUBLE)
    {
      _double_state = 3;
    }
    else if(blue.duration > LONG_DOUBLE)
    {
      _double_state = 2;
    }
  }

  if(_last_double != _double_state) //check if state has changed, 
  {
    _last_double = _double_state;
    _double_press = _double_state;
    if(_double_state == 0); //reset the LEDs in case we are backing down from a flash reset
    {
      blue.on(0x00);
      gold.on(READY_BLINK);
    }
  }
  else
  {
    _double_press = 0;
  }

  // check if blue button pressed
  if (_last_blue != blue.state) //see if change
  {
    if(blue.duration > PRESS_DELAY) //give it 75 ms before we count it
    {
      _last_blue = blue.state;
      if(blue.state)
      {
        _blue_pressed = true;
      }
    }
  }
  else //if no change in input, ignore it
  {
    _blue_pressed = false;
  }

  // check if gold button pressed
  if (_last_gold != gold.state) //see if change
  {
    if(gold.duration > PRESS_DELAY) //give it 75 ms before we count it
    {
      _last_gold = gold.state;
      if(gold.state)
      {
        _gold_pressed = true;
      }
    }
  }
  else //if no change in input, ignore it
  {
    _gold_pressed = false;
  }

  //check if input buttons were pressed
  for(uint8_t i = 0; i < 8; ++i) //check each button!
  {
    uint8_t ibstate = digitalRead(_input_buttons[i]);
    if( ((_last_input & (1<<i))>>i) != ibstate) //see if change
    {
      if(ibstate == HIGH)
      {
        _last_input |= (1<<i);
        _input_pressed |= (1<<i);
      }
      else
      {
        _last_input &= ~(1<<i);
      }
    }
    else //no change in input, ignore it
    {
      _input_pressed &= ~(1<<i);
    }
  }


  //possibilities: Blue and Gold was pressed, only Blue was pressed or only Gold was pressed, and/or an input button was pressed.
  uint8_t channel, prev_channel;
  //blue has been pressed.
  switch(_state)
  {
  case BG_IDLE:
    if(_double_press == 3) //8 seconds each
    {
      factoryReset();
    }
    else if(_double_press == 2) //3 seconds, begin factory reset warning
    {
      gold.on(0xAAAAAAAA);
      blue.on(0x55555555);
    }
    else if(_double_press == 1)
    {
      sendIdent();
    }
    else if(_blue_pressed)
    {
      //we were doing nothing before; a press of blue puts us in LEARN mode
      _state = BG_LEARN;
      //inhibit the EventHandler to avoid confusions
      _event_handler->inhibit();
      blue.on(0xF0F0F0F0); //light the BLUE lamp solid for learning
      for(uint8_t i = 0; i < 8; ++i)
      {
        digitalWrite(i, LOW);
      }
    }
    else if(_gold_pressed)
    {
      //we were doing nothing before; a press of gold puts us in TEACH mode
      _state = BG_TEACH;
      //inhibit the EventHandler to avoid confusions
      _event_handler->inhibit(); //TODO is this right?
      gold.on(0xF0F0F0F0); //light the GOLD lamp solid for learning
      for(uint8_t i = 0; i < 8; ++i)
      {
        digitalWrite(i, LOW);
      }
    }
    break;
  case BG_LEARN:
    if(_double_press)
    {
      moveToIdle(true);
    }
    else if(_blue_pressed)
    {
      gold.on(0);
      //we've entered learn state, now we're indexing over the outputs
      _index = ((_index+2)%33)-1;
      if(_index == -1) //cycled through, return to beginning.
      {
        digitalWrite(7, LOW); //turn off last channel.
        moveToIdle(true);
      }
      else if(_index > 15)
      {
        gold.on(0x0AAA000A);
        channel = (_index-16) >> 1;
        // if _index is even, we are handling the consumer for the output being off; blink the blue LED to indicate
        if(_index & 0x01)
        {
          blue.on(0xA000A000);
        }
        else
        {
          blue.on(0xFFFFFFFF);
        }
        digitalWrite(channel, HIGH);
        if(_index == 16)
        {
          digitalWrite(7, LOW);
        }
        else if(_index>1)
        {
          digitalWrite(channel-1, LOW); //turn off previous channel
        }
      }
      else
      {
        channel = _index >> 1;
        // if _index is even, we are handling the producer for the output being off; blink the blue LED to indicate
        if(_index & 0x01)
        {
          blue.on(0x000A000A);
        }
        else
        {
          blue.on(0xFFFFFFFF);
        }
        digitalWrite(channel, HIGH);
        if(_index>1)
        {
          digitalWrite(channel-1, LOW); //turn off previous channel
        }
      }
    }
    else if(_gold_pressed)
    {
      //send off the LEARN messages!
      //producers first
      if(_input_index > -1)
        _event_handler->markToLearn(_input_index, true);
      //consumers second
      if(_index > 15)
        _event_handler->markToLearn(_index-16, true);
      else if(_index > -1)
        _event_handler->markToLearn(_index+16, true);
      moveToIdle(false);
    }
    else if(_input_pressed)
	{
      //figure out which input was pressed
      uint8_t in;
      for(in = 0; in < 8; ++in)
      {
        if((_input_pressed) & (1<<in))
          break;
      }
      //check to see if this is first, second, or third time.
      //First time, we flag "on" producer for that channel
      //Second time, we unflag "on", flag "off"
      //Third time, we unflag "off"
      //check "on"
      //first, check to see if user has moved to a different input, changing their mind.
      if((_input_index == -1) || (in != (_input_index>>1)))
      {
      	gold.on(0x000A000A);
        _input_index = (in << 1);
        blue.on(0xFFFFFFFF);
        _input_count = 1;
      }
      else if(_input_count == 1)
      {
        _input_index = (in << 1) + 1;
        blue.on(0x000A000A); //indicate that "off" is selected
		_input_count = 2;
      }
      else if(_input_count == 2)
      {
      	gold.on(0);
        _input_index = -1;
        _input_count = 0;
        blue.on(0xF0F0F0F0); //indicate that nothing is selected for learning
      }
    }
    break;
  case BG_TEACH: //we've entered teach state, now we're indexing over the outputs
    if(_double_press)
    {
      moveToIdle(true);
    }
    else if(_blue_pressed)
    {
      gold.on(0xF0F0F0F0);
      _index = ((_index+2)%33)-1;
      if(_index == -1) //cycled through, return to beginning.
      {
        digitalWrite(7, LOW); //turn off last channel.
        blue.on(0xF0F0F0F0);
        //the difference is that we don't leave BG_TEACH state when we've wrapped around the outputs
      }
      else if(_index > 15)
      {
        gold.on(0x0AAA000A);
        channel = (_index-16) >> 1;
        // if _index is even, we are handling the consumer for the output being off; blink the blue LED to indicate
        if(_index & 0x01)
        {
          blue.on(0xA000A000);
        }
        else
        {
          blue.on(0xFFFFFFFF);
        }
        digitalWrite(channel, HIGH);
        if(_index == 16)
        {
          digitalWrite(7, LOW);
        }
        else if(_index>1)
        {
          digitalWrite(channel-1, LOW); //turn off previous channel
        }
      }
      else
      {
        channel = _index >> 1;
        // if _index is even, we are handling the producer for the output being off; blink the blue LED to indicate
        if(_index & 0x01)
        {
          blue.on(0xA000A000);
        }
        else
        {
          blue.on(0xFFFFFFFF);
        }
        digitalWrite(channel, HIGH);
        if(_index>1)
        {
          digitalWrite(channel-1, LOW); //turn off previous channel
        }
      }
    }
    else if(_gold_pressed)
    {
      //send off the TEACH messages!
      //producers first
      if(_input_index > -1)
        _event_handler->markToTeach(_input_index, true);
      //consumers second
      if(_index > 15)
        _event_handler->markToTeach(_index-16, true);
      else if(_index > -1)
        _event_handler->markToTeach(_index+16, true);
      moveToIdle(false);

    }
else if(_input_pressed)
{
	//ignore button-up events
    if(!(_input_pressed & 0x01))
    {
      //figure out which input was pressed
      uint8_t in;
      for(in = 0; in < 8; ++in)
      {
        if((_input_pressed) & (1<<in))
          break;
      }
      //check to see if this is first, second, or third time.
      //First time, we flag "on" producer for that channel
      //Second time, we unflag "on", flag "off"
      //Third time, we unflag "off"
      //check "on"
      //first, check to see if user has moved to a different input, changing their mind.
      if((_input_index == -1) || (in != (_input_index>>1)))
      {
      	gold.on(0x000A000A);
        _input_index = (in << 1);
        blue.on(0xFFFFFFFF);
        _input_count = 1;
      }
      else if(_input_count == 1)
      {
        _input_index = (in << 1) + 1;
        blue.on(0x000A000A); //indicate that "off" is selected
		_input_count = 2;
      }
      else if(_input_count == 2)
      {
      	gold.on(0);
        _input_index = -1;
        _input_count = 0;
        blue.on(0xF0F0F0F0); //indicate that nothing is selected for learning
      }
    }
}

    break;
  }
}

bool MyBlueGoldHandler::handleMessage(OLCB_Buffer *buffer)
{
  if(isPermitted())
  {
    return OLCB_Virtual_Node::handleMessage(buffer);
  }
  return false;
}

/**
 * Send an event in response to the "ident" button pushes
 */
void MyBlueGoldHandler::sendIdent()
{
  _link->sendIdent();
}

/**
 * Fire factory reset
 * ToDo: better name!  Not really a true "factory reset"
 */
void MyBlueGoldHandler::factoryReset()
{
  _event_handler->factoryReset();
  delay(100); //just because. Don't know if we need it
  // reboot
  // cast a 0 to a function pointer, then dereference it. Ugly!
  (*  ((void (*)())0)  )();
}

