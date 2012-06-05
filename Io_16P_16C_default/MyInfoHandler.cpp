#include "MyInfoHandler.h"
#include <OLCB_CAN_Buffer.h>
#include <avr/pgmspace.h>

//NOTE This class is largely a hack. It works, but there's a lot going on here that should be handled by the lower-level libraries!

#define MTI_SNIP_REQUEST  0x52
#define MTI_SNIP_RESPONSE 0x53
#define MTI_PIP_REQUEST   0x2E
#define MTI_PIP_RESPONSE  0x2F

static PROGMEM char snipstring[] = "\x01Railstars Limited\nIo Developer\'s Board\n1.0\n1.1";

bool isSNIPRequest(OLCB_Buffer *buffer)
{
  if(! (buffer->isFrameTypeOpenLcb() && (buffer->getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_MESSAGE)) )
    return false;
  return (buffer->data[0] == ((MTI_SNIP_REQUEST)&0xFF) );
}

bool isPIPRequest(OLCB_Buffer *buffer)
{
  if(! (buffer->isFrameTypeOpenLcb() && (buffer->getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_MESSAGE)) )
    return false;
  return (buffer->data[0] == ((MTI_PIP_REQUEST)&0xFF) );
}

void MyInfoHandler::update(void)
{
  if(!isPermitted())
  {
      return;
  }
  //handle pending snip request responses
  if(_string_index > -1)
  {
    //Serial.println("Working on SNIP response!");
    //we have work to do.
    //figure out how many bytes to send. Either 7 bytes, or whatever remains
    _reply.length = _string_length-_string_index +1; //the +1 is to include the datagram MTI
    if(_reply.length > 8)
      _reply.length = 8;
    //Serial.print("len = ");
    //Serial.println(_reply.length, DEC);
    //Serial.print("start index= ");
    //Serial.println(_string_index, DEC);
    for(uint8_t i = 1; i < _reply.length; ++i) //skip the MTI
    {
      _reply.data[i] = pgm_read_byte(snipstring+i+_string_index-1);
      if(_reply.data[i] == '\n')
        _reply.data[i] = 0;
      //Serial.print((char)(_reply.data[i]));
    }
    //Serial.println();
    while(! _link->sendMessage(&_reply) );
    //now, set up for next run.
    //Serial.println("setting up for next time");
    _string_index += (_reply.length-1);
    //Serial.print("new string index = ");
    //Serial.println(_string_index, DEC);
    //Serial.print(_string_index > strlen(_buffer));
    if(_string_index >= _string_length) //done!
    {
      //Serial.println("done!");
      _string_index = -1;
    }
  }
  else if(_messageReady)
  {
    _messageReady = false;
    while(! _link->sendMessage(&_reply));
  }
  OLCB_Virtual_Node::update();
}


void MyInfoHandler::create(OLCB_Link *link, OLCB_NodeID *nid)
{
  _string_index = -1;
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
    buffer->getDestinationNID(&dest);
    if(dest == *NID)
    {
      //Serial.println("Got SNIP request");
      _string_index = 0;
      OLCB_NodeID source_address;
      buffer->getSourceNID(&source_address);
      _reply.init(NID);
      _reply.setDestinationNID(&source_address);
      _reply.setFrameTypeOpenLcb();
      _reply.setOpenLcbFormat(MTI_FORMAT_ADDRESSED_MESSAGE);
      _reply.data[0] = MTI_SNIP_RESPONSE;
      retval = true;
    }
  }
  else if(isPIPRequest(buffer))
  {
    //is it for us?
    //Serial.println("got PIP aimed at.");
    OLCB_NodeID dest;
    buffer->getDestinationNID(&dest);
    //Serial.println(dest.alias, DEC);
    //Serial.println(NID->alias, DEC);
    if(dest == *NID)
    {
      //Serial.println("Got PIP request");
      OLCB_Buffer reply;
      OLCB_NodeID source_address;
      buffer->getSourceNID(&source_address);
      reply.init(NID);
      reply.setDestinationNID(&source_address);
      reply.setFrameTypeOpenLcb();
      reply.setOpenLcbFormat(MTI_FORMAT_ADDRESSED_MESSAGE);
      reply.data[0] = MTI_PIP_RESPONSE;
      reply.length = 7;
      reply.data[1] = 0x80 | 0x40 | 0x10 | 0x04 | 0x01;
      reply.data[2] = 0x00;
      reply.data[3] = 0x00;
      reply.data[4] = 0x00;
      reply.data[5] = 0x00;
      reply.data[6] = 0x00;
//      while(!_link->sendMessage(&reply)); //TODO THIS IS NOT PERMITTED!!
      _messageReady = true;
      retval = true;
    }
  }

  return retval;
}


