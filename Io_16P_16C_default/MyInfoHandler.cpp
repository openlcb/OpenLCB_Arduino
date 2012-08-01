#include "MyInfoHandler.h"
#include <OLCB_CAN_Buffer.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

//NOTE This class is largely a hack. It works, but there's a lot going on here that should be handled by the lower-level libraries!

#define MTI_SNIP_REQUEST  0x52
#define MTI_SNIP_RESPONSE 0x53
#define MTI_PIP_REQUEST   0x2E
#define MTI_PIP_RESPONSE  0x2F

static PROGMEM char snipstring[] = "\x01Railstars Limited\nIo Developer\'s Board\n1.0\n1.3";

bool isSNIPRequest(OLCB_Buffer *buffer)
{
  return buffer->isSNIIRequest();
}

bool isPIPRequest(OLCB_Buffer *buffer)
{
  return buffer->isProtocolSupportInquiry();
}

void MyInfoHandler::update(void)
{
  if(!isPermitted())
  {
      return;
  }
  //_reply.length = 1; //always at least one, for MTI byte
  //handle pending snip request responses
  if(_string_index > -1)
  {
    //Serial.println("Working on SNIP response!");
    //we have work to do.
    //figure out how many bytes to send. Either 7 bytes, or whatever remains
    _reply.length = _string_length-_string_index + 2;
    if(_reply.length > 8)
      _reply.length = 8;
    //Serial.print("len = ");
    //Serial.println(_reply.length, DEC);
    //Serial.print("start index= ");
    //Serial.println(_string_index, DEC);
    for(uint8_t i = 2; i < _reply.length; ++i) //skip the MTI
    {
      _reply.data[i] = pgm_read_byte(snipstring+i+_string_index-2);
      if(_reply.data[i] == '\n')
        _reply.data[i] = 0;
      //Serial.print((char)(_reply.data[i]));
    }
    //Serial.println();
    while(! _link->sendMessage(&_reply) );
    //now, set up for next run.
    //Serial.println("setting up for next time");
    _string_index += (_reply.length-2);
    //Serial.print("new string index = ");
    //Serial.println(_string_index, DEC);
    //Serial.print(_string_index > strlen(_buffer));
    if(_string_index >= _string_length) //done! with this part; now moving to EEPROM
    {
      _string_index = -1;
      _eeprom_index = 1027;
    }
  }
  //else, we're sending the second set of strings.
  else if(_eeprom_index > -1)
  {
      _reply.length = 2;
  	if((_eeprom_index == 1027) && (_reply.length < 8)) //1027 is a stand in to indicate to send the "01" byte first
  	{
        //Serial.println("start on user strings");
  		_reply.data[2] = 0x01;
    	        ++_reply.length;
  		++_eeprom_index;
  		_eeprom_string_index = 1;
  	}
  	if(_eeprom_string_index == 1) //user name
  	{
  		//read bytes until the reply is full, or we encounter a 0x00'
  		uint8_t c = 0xFF;
  		while( (_reply.length < 8) && (c != 0x00) && (_eeprom_index < 1060) )
  		{
  			c = EEPROM.read(_eeprom_index);
                        //Serial.print(c, HEX);
  			_reply.data[_reply.length] = c;
  			++_reply.length;
  			++_eeprom_index;
  		}
  		if( (c == 0x00) || (_eeprom_index >= 1060) ) //ready to move to next string
  		{
  			_eeprom_index = 1060;
  			_eeprom_string_index = 2;
                        //Serial.println();
  		}
  	}
  	if(_eeprom_string_index == 2) //user description
  	{
  	  	//read bytes until the reply is full, or we encounter a 0x00'
  		uint8_t c = 0xFF;
  		while( (_reply.length < 8) && (c != 0x00) && (_eeprom_index < 1188) )
  		{
  			c = EEPROM.read(_eeprom_index);
                        //Serial.print(c, HEX);
  			_reply.data[_reply.length] = c;
  			++_reply.length;
  			++_eeprom_index;
  		}
  		if( (c == 0x00) || (_eeprom_index >= 1188) ) //ready to be done
  		{
  			_eeprom_index = -1;
   			_eeprom_string_index = 0;
                        //Serial.println();
  		}
  	}
  	while(! _link->sendMessage(&_reply) );

  }
  else if(_messageReady)
  {
    //Serial.println("Found a message to be delivered!");
    _messageReady = false;
    //Serial.println(_reply.data[0], HEX);
    //Serial.println(_reply.flags.extended, HEX);
    //Serial.println(_reply.id, HEX);
    while(! _link->sendMessage(&_reply));
  }
  OLCB_Virtual_Node::update();
}


void MyInfoHandler::create(OLCB_Link *link, OLCB_NodeID *nid)
{
  _messageReady = false;
  _string_index = -1;
  _eeprom_index = -1;
  _eeprom_string_index = 0;
  _string_length = strlen_P(snipstring)+1; //the first +1 is to INCLUDE the null;
  OLCB_Virtual_Node::create(link,nid);
}

bool MyInfoHandler::handleMessage(OLCB_Buffer *buffer)
{
  //we care about the following kinds of messages:
  //Protocol Identification Protocol (PIP)
  //Simple Node Identification Protocol (SNIP)
  if(!isPermitted())
  {
    return false;
  }
  
  bool retval = false;

  if(isSNIPRequest(buffer))
  {
    //is it for us?
    OLCB_NodeID dest;
    buffer->getDestNID(&dest);
    if(dest == *NID)
    {
      //Serial.println("Got SNIP request");
      if( (_string_index > -1) || (_eeprom_index > -1) ) //we're busy!
      {
        //Serial.println("cant handle SNIP/PIP request, returning false");
        retval = false;
      }
      else
      {
        //send a SNII Re
        _string_index = 0;
        OLCB_NodeID source_address;
        buffer->getSourceNID(&source_address);
        _reply.setSNIIReply(NID, &source_address);
        retval = true;
      }
    }
  }
  else if(isPIPRequest(buffer))
  {
    //is it for us?
    //Serial.println("got PIP aimed at.");
    OLCB_NodeID dest;
    buffer->getDestNID(&dest);
    //Serial.println(dest.alias, DEC);
    //Serial.println(NID->alias, DEC);
    if(dest == *NID)
    {
      //Serial.println("Got PIP request");
      OLCB_NodeID source_address;
      buffer->getSourceNID(&source_address);
      _reply.setProtocolSupportReply(NID, &source_address);
      _reply.length = 8; //first two bytes are destination alias
      _reply.data[2] = 0x80 | 0x40 | 0x10 | 0x04 | 0x01;
      _reply.data[3] = 0x10 | 0x08;
      _reply.data[4] = 0x00;
      _reply.data[5] = 0x00;
      _reply.data[6] = 0x00;
      _reply.data[7] = 0x00;
      //for(uint8_t i = 0; i < _reply.length; ++i)
        //Serial.println(_reply.data[i], HEX);
      _messageReady = true;
      retval = true;
    }
  }
  return retval;
}


