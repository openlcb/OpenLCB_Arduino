#include "MyConfigHandler.h"
#include "EepromLayout.h"
#include <reset.h>
#include <EEPROM.h>
#include "Logging.h"

void MyConfigHandler::create(OLCB_Link *link, OLCB_NodeID *nid, MyEventHandler *eventHandler)
{
  _eventHandler = eventHandler;
  OLCB_Datagram_Handler::create(link,nid);
}


void MyConfigHandler::datagramResult(bool accepted, uint16_t errorcode)
{
  if(!accepted)
  {
  }
}

void MyConfigHandler::initialize(void)
{
  return;
}

uint32_t MyConfigHandler::getAddress(uint8_t* data)
{
  uint32_t val = 0;
  val |= ((uint32_t)data[2]<<24);
  val |= ((uint32_t)data[3]<<16);
  val |= ((uint32_t)data[4]<<8);
  val |= ((uint32_t)data[5]);
  return val;
}

uint8_t MyConfigHandler::decodeSpace(uint8_t* data) {
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

// -1 means stream
uint8_t MyConfigHandler::decodeLength(uint8_t* data)
{
  return data[6];
}

uint16_t MyConfigHandler::processDatagram(void)
{
  //To have made it this far, we can be sure that _rxDatagramBuffer has a valid datagram loaded up, and that it is in fact addressed to us.

  if(!isPermitted()) //only act on it if we are in Permitted state. Otherwise no point.
  {
    return DATAGRAM_REJECTED;
  }
    DebugSerial.println("GOT A DATAGRAM!");
    DebugSerial.println(_rxDatagramBuffer->length, DEC);
    for(uint8_t i = 0; i < _rxDatagramBuffer->length; ++i)
      DebugSerial.println(_rxDatagramBuffer->data[i], HEX);
    //check the first byte of the payload to see what kind of datagram we have
    switch(_rxDatagramBuffer->data[0])
    {
    case MAC_PROTOCOL_ID: //MAC protocol
      DebugSerial.println("using MAC protocol");
      switch (_rxDatagramBuffer->data[1]&0xC0)
      {
      case MAC_CMD_READ:
        DebugSerial.println("read request");
        return MACProcessRead();
        break;
      case MAC_CMD_WRITE:
        DebugSerial.println("write request");
        return MACProcessWrite();
        break;
      case MAC_CMD_OPERATION:
        DebugSerial.println("cmd request");
        return MACProcessCommand();
        break;
      }
    }

  DebugSerial.println("Not for us to handle, going to NAK");
  return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;
}

//Memory Access Configuration Protocol
uint16_t MyConfigHandler::MACProcessRead(void)
{
  uint32_t address = getAddress(_rxDatagramBuffer->data);
  uint8_t space = decodeSpace(_rxDatagramBuffer->data);
  uint8_t datagram_offset = (_rxDatagramBuffer->data[1]&0x03)?6:7; //if the space is encoded in the flags in data[1], then the payload begins at byte 6; else, the payload begins after the space byte, at byte 7.
  uint8_t length = _rxDatagramBuffer->data[datagram_offset];
 
  
  OLCB_Datagram reply;
  memcpy(&(reply.destination), &(_rxDatagramBuffer->source), sizeof(OLCB_NodeID));
  //copy first six/seven bytes.
  memcpy(reply.data, _rxDatagramBuffer->data, datagram_offset);
  reply.data[1] = MAC_CMD_READ_REPLY | reply.data[1]&0x0F; //set as read response
  DebugSerial.println("MACPROCESSREAED");
  DebugSerial.print("Making reply with MTI ");
  DebugSerial.println(reply.data[0], HEX);
  DebugSerial.println(reply.data[1], HEX);
  //TODO presume datagram?
  DebugSerial.print("length = ");
  DebugSerial.println(length, DEC);
  DebugSerial.print("address = ");
  DebugSerial.println(address, HEX);
  DebugSerial.print("space = ");
  DebugSerial.println(space, HEX);
  DebugSerial.print("data offset in datagram = ");
  DebugSerial.println(datagram_offset);
  //And, now do something useful.
  //first check the space?
  switch(space)
  {
    case 0x01: //EEPROM
      reply.length = length+datagram_offset;
      for(uint8_t i = 0; i < length; ++i)
      {
         reply.data[datagram_offset+i] = EEPROM.read(address+i);
      }
    if(sendDatagram(&reply))
      return DATAGRAM_ERROR_OK;
    else
      //have them resend
      return DATAGRAM_REJECTED_BUFFER_FULL;
    
  case 0xFF: //CDI request.
    DebugSerial.println("CDI request.");
    reply.length = readCDI(address, length, &(reply.data[datagram_offset])) + datagram_offset;
    DebugSerial.print("total length of CDI reply = ");
    DebugSerial.println(reply.length, DEC);
    if(sendDatagram(&reply))
      return DATAGRAM_ERROR_OK;
    else
      //have them resend
      return DATAGRAM_REJECTED_BUFFER_FULL;
  case 0xFE: //"All memory" access. Just give them what they want?
    //DebugSerial.println("all memory request. ignoring");
    break;
  case 0xFD: //configuration space
    //DebugSerial.println("configuration space.");
  	if(address < EE_EVENT_DESCRIPTION_START)
  	{
  		reply.length = _eventHandler->readConfig(address, length, &(reply.data[datagram_offset])) + datagram_offset;
	}
	else
	{
		reply.length = length + datagram_offset; //give them what they want
		for(uint16_t i = 0; i < length; ++i)
		{
			//DebugSerial.print("Read: ");
		        //DebugSerial.print(address+i, HEX);
                        //DebugSerial.print(" ");
                        //DebugSerial.print(i+6, HEX);
			//DebugSerial.print(":");
			uint8_t x = EEPROM.read(address+i);
                        //DebugSerial.println(x, HEX);
			reply.data[datagram_offset+i] = x;
		}
	}
    DebugSerial.print("Outbound Datagram len: ");
    DebugSerial.println(reply.length, DEC);
    if(sendDatagram(&reply))
    {
      //DebugSerial.println("away OK");
      return DATAGRAM_ERROR_OK;
    }
    else
    {
      //DebugSerial.println("Failure: TX Buffer full");
      //have them resend
      return DATAGRAM_REJECTED_BUFFER_FULL;
    }
  }
  //DebugSerial.println("NAKing");
  return DATAGRAM_REJECTED; //send a NAK. Is this what we really want?
}

uint16_t MyConfigHandler::MACProcessWrite(void)
{
  uint32_t address = getAddress(_rxDatagramBuffer->data);
  uint8_t space = decodeSpace(_rxDatagramBuffer->data);
  uint8_t datagram_offset = (_rxDatagramBuffer->data[1]&0x03)?6:7; //if the space is encoded in the flags in data[1], then the payload begins at byte 6; else, the payload begins after the space byte, at byte 7.
  uint8_t length = (_rxDatagramBuffer->length)-datagram_offset;
  
  //DebugSerial.println("Write Command");
  //DebugSerial.print("length =");
  //DebugSerial.println(length, HEX);
  //DebugSerial.print("address =");
  //DebugSerial.println(address, HEX);
  //DebugSerial.print("space =");
  //DebugSerial.println(space, HEX);
  
  //And, now do something useful.
  //first check the space?
  switch(space)
  {
  case 0x01: //EEPROM
    for(uint8_t i = 0; i < length; ++i)
    {
      if( (address+i) < 0x0FF8)
         EEPROM.write(address+i, _rxDatagramBuffer->data[datagram_offset+i]);
    }
    return DATAGRAM_ERROR_OK;
  case 0xFF: //CDI request.
    //TODO
    break;
  case 0xFE: //"All memory" access. Just give them what they want?
    //TODO
    break;
  case 0xFD: //configuration space
  	//send to eventHandler, if less than 260, as it will need to update the event table directly. Otherwise, it's ours to handle
  	if(address < EE_EVENT_DESCRIPTION_START)
  	{
            //DebugSerial.println("sending to EventHandler");
	    _eventHandler->writeConfig(address, length, &(_rxDatagramBuffer->data[datagram_offset]));
	}
	else
	{
		for(uint16_t i = 0; i < length; ++i)
		{
			//DebugSerial.print("Write: ");
			//DebugSerial.print(address+i, HEX);
			//DebugSerial.print(":");
			//DebugSerial.println(_rxDatagramBuffer->data[6+i], HEX);
			EEPROM.write(address+i, _rxDatagramBuffer->data[datagram_offset+i]);
		}
	}
    return DATAGRAM_ERROR_OK;
  }
  return DATAGRAM_REJECTED; //send a NAK. Is this what we really want?
}

uint16_t MyConfigHandler::MACProcessCommand(void)
{
  OLCB_Datagram reply;
  reply.data[0] = MAC_PROTOCOL_ID;
  memcpy(&(reply.destination), &(_rxDatagramBuffer->source), sizeof(OLCB_NodeID));
  reply.length = 0; //a flag to know if we are handling this message
  switch (_rxDatagramBuffer->data[1]&0xFC)
  {
  case MAC_CMD_GET_CONFIG_OPTIONS:
    DebugSerial.println("MAC_CMD_GET_CONFIG_OPTIONS");
    reply.length = 7;
    reply.data[1] = MAC_CMD_GET_CONFIG_OPTIONS_REPLY;
    reply.data[2] = MAC_CONFIG_OPTIONS_UNALIGNED_READS | MAC_CONFIG_OPTIONS_UNALIGNED_WRITES | MAC_CONFIG_OPTIONS_MFG_ACDI_FD_READ | MAC_CONFIG_OPTIONS_USR_ACDI_FC_READ; //available commands byte 1
    reply.data[3] = 0x00; //available commands byte 2
    reply.data[4] = MAC_CONFIG_OPTIONS_1_BYTE_WRITE | MAC_CONFIG_OPTIONS_2_BYTE_WRITE | MAC_CONFIG_OPTIONS_4_BYTE_WRITE | MAC_CONFIG_OPTIONS_64_BYTE_WRITE | MAC_CONFIG_OPTIONS_ARBITRARY_WRITE;
    reply.data[5] = 0xFF; //highest address space
    reply.data[6] = 0xFD; //lowest address space
    break;
  case MAC_CMD_GET_ADD_SPACE_INFO:
    DebugSerial.println("MAC_CMD_GET_ADD_SPACE_INFO");
    reply.length = 8; //minimally!
    reply.data[1] = MAC_CMD_GET_ADD_SPACE_INFO_REPLY;
    reply.data[2] = _rxDatagramBuffer->data[2];
    reply.data[3] = 0x00; //largest address
    reply.data[4] = 0x00;
    reply.data[5] = 0x00;
    reply.data[6] = 0x00; //largest address continued
    reply.data[7] = 0x01; //flags
    switch(_rxDatagramBuffer->data[2])
    {
    case 0xFF: //CDI
      reply.data[1] |= 0x01; //present
      reply.data[3] = (uint32_t)(sizeof(cdixml))>>24 & 0xFF;
      reply.data[4] = (uint32_t)(sizeof(cdixml))>>16 & 0xFF;
      reply.data[5] = (uint32_t)(sizeof(cdixml))>>8 & 0xFF;
      reply.data[6] = (uint32_t)(sizeof(cdixml)) & 0xFF;
      break;
    case 0xFE: //"all memory"
      break;
    case 0xFD: //configuration space
      reply.data[1] |= 0x01; //present
      //largest address is.
      reply.data[3] = _eventHandler->getLargestAddress()>>24 & 0xFF;
      reply.data[4] = _eventHandler->getLargestAddress()>>16 & 0xFF;
      reply.data[5] = _eventHandler->getLargestAddress()>>8 & 0xFF;
      reply.data[6] = _eventHandler->getLargestAddress() & 0xFF;
      reply.data[7] = 0x00; //can write
      break;
    }
  case MAC_CMD_RESETS:
    DebugSerial.println("MAC_CMD_RESETS");
    // force restart (may not reply?)
    if ((_rxDatagramBuffer->data[1]&0x03) == 0x01)
    { // restart/reboot?
      DebugSerial.println("restart");
      //TODO tell other handlers that we need to write out anything that needs to be saved!!!
//        cli();
//        ((void (*)(void))0xF000)();
      soft_reset();
    }
    else if((_rxDatagramBuffer->data[1]&0x03) == 0x02) //factory reset!
    {
      DebugSerial.println("factory reset");
      //check NID first!
      OLCB_NodeID n;
      memcpy(&n, &(_rxDatagramBuffer->data[2]), 6);
      if(n.sameNID(NID))
      {
        _eventHandler->factoryReset();
        //cli();
        //((void (*)(void))0xF000)();
        soft_reset();
      }
    }
    // TODO: Handle other cases
    break;
  case MAC_CMD_LOCK:
    DebugSerial.println("MAC_CMD_LOCK");
    break;
  case MAC_CMD_GET_UNIQUEID:
    DebugSerial.println("MAC_CMD_GET_UNIQUEID");
    {
      uint8_t num = (_rxDatagramBuffer->data[2] & 0x07);
      reply.length = 2+(8*num);
      reply.data[1] = MAC_CMD_GET_UNIQUEID_REPLY;
      uint8_t eid6 = EEPROM.read(2);
      uint8_t eid7 = EEPROM.read(3);
      for(uint8_t i = 0; i < num; ++i)
      {
        for(uint8_t j = 0; i < 6; ++j)
        {
          reply.data[(i*8)+j+2] = NID->val[j];
        }
        reply.data[(i*8)+7+2] = eid6;
        reply.data[(i*8)+8+2] = eid7;
        if(eid7 == 254) //need to increment val[6] as well
        {
          eid6++; //might wrap around to 0; it's gonna happen, I guess
        }
        eid7++;
      }
      EEPROM.write(2,eid6);
      EEPROM.write(3,eid7);
    }
    break;
  case MAC_CMD_FREEZE:
    DebugSerial.println("MAC_CMD_FREEZE");
    break;
  case MAC_CMD_INDICATE:
    DebugSerial.println("MAC_CMD_INDICATE");
    break;
  default:
    DebugSerial.println("Not recognized");
    break;
  }

  if(reply.length)
  {
    if(sendDatagram(&reply))
      return DATAGRAM_ERROR_OK;
    else
      return DATAGRAM_REJECTED_BUFFER_FULL;
  }
  else //we didn't handle the datagram
    return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;

  return DATAGRAM_REJECTED;
}

uint8_t MyConfigHandler::readCDI(uint16_t address, uint8_t length, uint8_t *data)
{
  //This method gets called by configuration handlers. We are being requested for the CDI file, a chunk at a time.
  //DebugSerial.println("readCDI");
  //DebugSerial.print("length: ");
  //DebugSerial.println(length);
  uint16_t capacity = sizeof(cdixml)+1;
  if( (length+address) > (capacity) ) //too much! Would cause overflow
    //caculate a shorter length to prevent overflow
    length = capacity - address;
  //DebugSerial.print("modified length: ");
  //DebugSerial.println(length);
  //DebugSerial.print("Read from index ");
  //DebugSerial.println(address, DEC);
  //we can't do a straight memcpy, because xmlcdi is PROGMEM. So, we have to use read_bytes instead
  uint16_t i;
  for(i = 0; i < length; ++i)
  {
    if( (i+address) >= sizeof(cdixml) )
    {
      *(data+(i)) = 0;
    }
    else
    {
      uint8_t c = pgm_read_byte(&cdixml[i+address]);
      *(data+(i)) = c;
    }
  }
  //DebugSerial.println("===");
  //for(i = 0; i < length; ++i)
    //DebugSerial.println(*(data+i), HEX);
    return length;
}

