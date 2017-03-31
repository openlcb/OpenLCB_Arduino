#include "MyEventHandler.h"
#include "EepromLayout.h"
#include <EEPROM.h> 
//really, should probably be using avr/eeprom.h, but this is going to be more portable in the end, I think?


static void maybe_write(int ofs, uint8_t value) {
  if (EEPROM.read(ofs) == value) return;
  EEPROM.write(ofs, value);
}

bool MyEventHandler::store(void)
{
  //write the stored EventIDs into EEPROM
  //TODO just brute forcing it. Probably a better way!
  for(uint8_t i = 0; i < _numEvents; ++i)
  {
    for(uint8_t j = 0; j < 8; ++j)
    {
      maybe_write((i*8)+j+EE_EVENT_START_OFS, _events[i].val[j]);
    }
  }
  return true;
}

bool MyEventHandler::load(void)
{
  //read the stored EventIDs from EEPROM
  for(uint8_t i = 0; i < _numEvents; ++i)
  {
    for(uint8_t j = 0; j < 8; ++j)
    {
      _events[i].val[j] = EEPROM.read((i*8)+j+EE_EVENT_START_OFS);
    }
  }
  return true;
}

void MyEventHandler::initialize(OLCB_Event *events, uint8_t num)
{

  /****
   * Notes on EEPROM:
   * This class checks to see if the EEPROM has been programmed by checking two bytes, to see if they contain the string "Io". If not, we know that the EEPROM has not been formated, and the class proceeds by writing several things to EEPROM:
   * 	1) the string "Io"
   * 	2) the next available EventID, namely, NodeID.0.0, stored as two bytes
   * 	3) A set of 32 EventIDs
   * On the other hand, if the string "Io" is present, the EEPROM is read into memory
   ****/
  loadEvents(events, num);

  //first, check first two bytes:
  if( (EEPROM.read(0) != 'I') || (EEPROM.read(1) != 'o') ) //not formatted!
  {
    //Serial.println("EEPROM not formatted!");
    factoryReset();
  }
  else
  {
    //Serial.println("EEPROM already formatted");
  }
  //now, read it back out into SRAM
  load(); //TODO add check for valid EEPROM.

  //and a little more setup.first the 16 producers
  for(uint8_t i = 0; i < 16; ++i)
    newEvent(i, true, false);
  //and the 48 consumers
  for(uint8_t i = 16; i < 80; ++i)
    newEvent(i, false, true);
}

void MyEventHandler::factoryReset(void)
{
  // WARNING: THIS FUNCTION RESETS THE EVENT POOL TO FACTORY SETTINGS!
  //first, check to see if the EEPROM has been formatted yet.
  uint8_t i;
  if( (char(EEPROM.read(0)) != 'I') || (char(EEPROM.read(1)) != 'o') ) //not formatted
  {
    // Format eeprom first: add zero as next event.
    EEPROM.write(0x02, 0x00);
    EEPROM.write(0x03, 0x00);
    //And the formatted tag
    EEPROM.write(0x00, 'I');
    EEPROM.write(0x01, 'o');
  }

  //first, increment the next available ID by numEvents, and write it back
  unsigned next_ev = EEPROM.read(0x02);
  next_ev <<= 8;
  next_ev |= EEPROM.read(0x03);

  unsigned resulting_next_ev = next_ev + _numEvents;
  maybe_write(0x02, resulting_next_ev >> 8);
  maybe_write(0x03, resulting_next_ev & 0xff);

  //now write the new EventIDs
  for(int i = 0; i < _numEvents; ++i)
  {
    for(int j = 0; j < 6; ++j)
    {
      maybe_write((i*8)+j+EE_EVENT_START_OFS, NID->val[j]);
    }
    maybe_write((i*8)+6+EE_EVENT_START_OFS, next_ev >> 8);
    maybe_write((i*8)+7+EE_EVENT_START_OFS, next_ev & 0xff);
    ++next_ev;
  }

  //Third, the user strings for each event need to be initialized to all null chars
  for(uint16_t i = EE_EVENT_DESCRIPTION_START; i < (EE_EVENT_DESCRIPTION_START+16*(EE_EVENT_COUNT>>1)); ++i)
  {
    maybe_write(i, 0);
  }
  //Fourth, the user strings for the node as a whole need to be initialized to all null chars
  for(uint16_t i = EE_NODE_DESCRIPTION_START; i < (EE_NODE_DESCRIPTION_START+128); ++i)
  {
    maybe_write(i, 0);
  }
  
}

void MyEventHandler::update(void)
{
  if(!isPermitted())
  {
    return;
  }

  for(uint8_t i = 0; i < 8; ++i) {
    //first, are we going to set up any kind of learning? TODO

    if(!_inhibit && !_first_run)
    {
      // looks at the input pins and
      // and decide whether and which events to fire
      // with pce.produce(i);
      // inputs are pins 8..15
      uint8_t state, prev_state;
      for(uint8_t i = 0; i < 8; ++i)
      {
        state = digitalRead(_input_buttons[i]); //digitalRead(i+8);
        prev_state = (_inputs & (1<<i))>>i;
        if((state != prev_state) || _first_check) //change in state!
        {	//input has changed, fire event and update flag
          //Serial.print("input state change to ");
          //Serial.print(state, DEC);
          //Serial.print(" on ");
          //Serial.println(i, DEC);
          //Serial.println(_inputs, BIN);
          _inputs ^= (1<<i); //toggle flag
          //now determine which producer to fire.
          //Serial.print("Producing ");
          //Serial.println((i<<1) | !(state^0x01), HEX);
          produce((i<<1) | !(state^0x01));
        }
      }
      _first_check = false;
    }
  }
  OLCB_Event_Handler::update(); //called last to permit the new events to be sent out immediately.
  if(_sendEvent == _numEvents) {
    _first_run = false;
  }
  if(_dirty) //check to see if we need to dump memory to EEPROM
  {
    store();
    _dirty = 0;
  }
}

bool MyEventHandler::consume(uint16_t index)
{
  if(_inhibit)
    return true;
  /* We've received an event; let's see if we need to consume it */
  //Serial.print("consume() ");
  //Serial.println(index,DEC);
  //Outputs are pins 0..7
  //odd events are off, even events are on
  if (index < 16) return false; // 0..7 are producers
  digitalWrite(_output_pins[(index-16)>>1], !(index&0x1));
  return true;
}


uint32_t MyEventHandler::getLargestAddress(void)
{
  return EE_NODE_DESCRIPTION_START + 128 - 1;
}

uint8_t MyEventHandler::readConfig(uint16_t address, uint8_t length, uint8_t *data)
{
  //This method gets called by configuration handlers. Basically, we are being asked for an EventID. We'll simply read from memory.
  //decode the address into a producer/consumer by dividing by 8
  //Serial.println("readConfig");
  //Serial.print("length: ");
  //Serial.println(length);
  uint8_t index = (address>>3);
  uint16_t offset = address - (index<<3) - 4;  // TODO: event offset here?
  if( (length+address-4) > (_numEvents*8) ) //too much! Would cause overflow
    //TODO caculate a shorter length to prevent overflow
    length = (_numEvents*8) - (address);
  //Serial.print("modified length: ");
  //Serial.println(length);
  //Serial.print("offset: ");
  //Serial.println(offset);
  //Serial.print("reading eventID from event pool index ");
  //Serial.println(index, DEC);
  //we can't do a straight memcpy, because EventIDs are actually NINE bytes long (the flags), and we want to skip every ninth byte.
  //so, we skip an address if it is even divisible by 8?
  uint8_t i, j, k;
  j = offset;
  k = index;
  for(i = 0; i < length; ++i, ++j)
  {
    if(j == 8)
    {
      j = 0;
      ++k;
    }
    *(data+i) = _events[k].val[j];
    //Serial.println(_events[k].val[j], HEX);
  }
  //Serial.println("===");
  //for(i = 0; i < length; ++i)
    //Serial.println(*(data+i), HEX);
  return length;
}

void MyEventHandler::writeConfig(uint16_t address, uint8_t length, uint8_t *data)
{
  //This method gets called by configuration handlers. We are being asked to write an EventID. We'll write it to memory, and do a lazy write to EEPROM later.

  //decode the address into a producer/consumer by dividing by 8
  uint8_t index = (address>>3);
  uint16_t offset = address - (index<<3) - 4;
  if( (length+address - 4) > (_numEvents*8) ) //too much! Would cause overflow
    //TODO caculate a shorter length to prevent overflow
    length = (_numEvents*8) - (address);
  //Serial.println("writeConfig");
  //Serial.print("length: ");
  //Serial.println(length);
  //Serial.print("offset: ");
  //Serial.println(offset);
  //Serial.print("writing eventID from event pool index ");
  //Serial.println(index, DEC);
  //we can't do a straight memcpy, because EventIDs are actually NINE bytes long (the flags), and we want to skip every ninth byte.
  //so, we skip an address if it is even divisible by 8?
  uint8_t i, j, k;
  j = offset;
  k = index;
  for(i = 0; i < length; ++i, ++j)
  {
    if(j == 8)
    {
      j = 0;
      ++k;
    }
    _events[k].val[j] = *(data+i);
    //Serial.println(_events[k].val[j], HEX);
  }
  //Serial.println("===");
  //for(i = 0; i < length; ++i)
  //Serial.println(*(data+i), HEX);
  _dirty = 1;
}


