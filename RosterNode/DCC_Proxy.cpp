// #ifdef DO_OLCB_DCC_TRAIN
#include <avr/eeprom.h>
#include <OpenLCB.h>
#include "DCC_Proxy.h"
#include "float16.h"
#include <reset.h>

//TODO need to include a generic handleMessage method for catching search messages!

    void DCC_Proxy::create(OLCB_Link *link, OLCB_NodeID *nid)
    {
      OLCB_Datagram_Handler::create(link, nid);
      _dcc_address = 0xFFFF;
      
      _timer = millis();
      _speed = 0;
	//DCC_Proxy_FX = 0;
//	uint8_t i;
//	for(i = 0; i < NUM_SIMULTANEOUS_CONTROLLERS; ++i)
//	{
//		DCC_Proxy_controllers[i] = NULL;
//	}
//	for(uint8_t i = 0; i < 127; ++i)
//	{
//		DCC_Proxy_speed_curve[i] = map(i, 1, 127, 0, 255);
	//	Serial.print(i);
	//	Serial.print(": ");
	//	Serial.println(DCC_Proxy_speed_curve[i]);
//	}
    }
    
    void DCC_Proxy::setDCCAddress(uint16_t new_address)
    {
      _dcc_address = new_address;
    }

void DCC_Proxy::update(void)
{
    if(!isPermitted())
      return;
      
	//see if we need to send out any periodic updates
//	uint32_t time = millis();
//	if( (time - _timer) >= 60000 ) //one minute
//	{
//          _timer = time;
//	  Serial.print("Loco at address ");
//          Serial.print(_dcc_address, DEC);
//          Serial.print(" periodic speed update to ");
//          Serial.println(_speed);
          //DCC_Controller->setSpeed(_dcc_address, _dcc_address_kind, _speed, _speed_steps);
//	}
      OLCB_Datagram_Handler::update();
}

  void DCC_Proxy::datagramResult(bool accepted, uint16_t errorcode)
  {
     //Serial.print("The datagram was ");
     //if(!accepted)
       //Serial.print("not ");
     //Serial.println("accepted.");
     //if(!accepted)
     //{
       //Serial.print("   The reason: ");
       //Serial.println(errorcode,HEX);
     //}
  }
  

/*
bool DCC_Proxy::DCC_Proxy_isAttached(OLCB_NodeID *node)
{
	for(uint8_t i = 0; i < NUM_SIMULTANEOUS_CONTROLLERS; ++i)
	{
		if( DCC_Proxy_controllers[i] && (*(DCC_Proxy_controllers[i]) == *node) )
			return true;
	}
	return false;
}
*/

uint16_t DCC_Proxy::processDatagram(OLCB_Datagram *datagram)
{		
	Serial.println("Got a datagram");
    if(!isPermitted()) // && (_rxDatagramBuffer->destination == *OLCB_Virtual_Node::NID))
      return DATAGRAM_REJECTED_PERMANENT_ERROR; // TODO NEED A WYA TO HANDLE THE UNPERMITTED CASE
      
      
    Serial.println("Got a datagram");
  
	//from this point on, it is for us!
	//make sure that it is a train control datagram, and handle that accordingly
if(!datagram->length) //check for zeo length
return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;

	switch(datagram->data[0])
        {
          case DATAGRAM_MOTIVE:
            return _handleTractionDatagram(datagram);
          case DATAGRAM_MEMCONFIG:
            return _handleMemConfigDatagram(datagram);
        }		
	Serial.println("not a motive datagram"); //TODO HANDLE CONFIGURATION DATAGRAMS!!
	return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;
}

uint16_t DCC_Proxy::_handleTractionDatagram(OLCB_Datagram *datagram)
{
  switch(datagram->data[1])
		{
		//case DATAGRAM_MOTIVE_ATTACH:
			//Serial.println("attach");
			//return handleAttachDatagram(datagram);
		//case DATAGRAM_MOTIVE_RELEASE:
			//Serial.println("release");
			//return handleReleaseDatagram(datagram);
		case DATAGRAM_MOTIVE_SETSPEED:
			Serial.println("setspeed");
			return _handleSetSpeedDatagram(datagram);
		case DATAGRAM_MOTIVE_GETSPEED:
			Serial.println("getspeed");
			return _handleGetSpeedDatagram(datagram);
		//case DATAGRAM_MOTIVE_SETFX:
			//return handleSetFXDatagram(datagram);
		//case DATAGRAM_MOTIVE_GETFX:
			//return handleGetFXDatagram(datagram);

	}
  return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;
}

uint16_t DCC_Proxy::_handleSetSpeedDatagram(OLCB_Datagram *datagram)
{
//	if(DCC_Proxy_isAttached(&(datagram->source)))
//	{
		//incoming speed is a signed float16
		//we store it as a signed 8-bit int, with -1/1 = stop (and 0 = estop), and -127/127 = max speed
		//notice that it is not enough to get the raw integral value of the float16, but we must scale it, because the DCC speed steps != absolute speed, but throttle notches. So we have to account for what motor speed each notch represents, and choose the appropriate notch..yuck!
		//finally, unhandled here, users can set a custom scale value. TODO
		_float16_shape_type f_val;
		Serial.println("Speed change");
		Serial.println("raw data");
		f_val.words.msw = datagram->data[2];
		f_val.words.lsw = datagram->data[3];
		Serial.print(datagram->data[2], HEX);
		Serial.print(" ");
		Serial.println(datagram->data[3], HEX);
		float new_speed = float16_to_float32(f_val);
		Serial.println(new_speed);
		Serial.println("----");
		_speed = map(new_speed, -100, 100, -125, 125);
		//TODO OTHER STUFF TOO! Send it on to CS!
	return DATAGRAM_ERROR_OK;
}

uint16_t DCC_Proxy::_handleGetSpeedDatagram(OLCB_Datagram *datagram)
{
	OLCB_Datagram reply;
	memcpy(&(reply.destination), &(_rxDatagramBuffer->source), sizeof(OLCB_NodeID));
	reply.data[0] = DATAGRAM_MOTIVE;
	reply.data[1] = DATAGRAM_MOTIVE_GETSPEED;
	float f_speed = map(_speed, -125, 125, -100, 100);
	_float16_shape_type f_val;
	f_val = float32_to_float16(f_speed);
	reply.data[2] = f_val.words.msw;
	reply.data[3] = f_val.words.lsw;
	reply.length = 4;
	reply.destination = datagram->source;
	if(sendDatagram(&reply))
		return DATAGRAM_ERROR_OK;
	//else
	//have them resend
	return DATAGRAM_REJECTED_BUFFER_FULL;
}


uint32_t DCC_Proxy::getAddress(uint8_t* data)
{
	uint32_t val = 0;
	val |= ((uint32_t)data[2]<<24);
	val |= ((uint32_t)data[3]<<16);
	val |= ((uint32_t)data[4]<<8);
	val |= ((uint32_t)data[5]);
	return val;
}

uint8_t DCC_Proxy::decodeSpace(uint8_t* data) {
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
uint8_t DCC_Proxy::decodeLength(uint8_t* data)
{
	return data[6];
}

uint16_t DCC_Proxy::_handleMemConfigDatagram(OLCB_Datagram* datagram)
{
		switch (datagram->data[1]&0xC0)
		{
			case MAC_CMD_READ:
			Serial.println("read request");
			return MACProcessRead();
			case MAC_CMD_WRITE:
			Serial.println("write request");
			return MACProcessWrite();
			case MAC_CMD_OPERATION:
			Serial.println("cmd request");
			return MACProcessCommand();
		}
	Serial.println("Uknown mem config datagram");
	return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;
}

//Memory Access Configuration Protocol
uint16_t DCC_Proxy::MACProcessRead(void)
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
	Serial.println("MACPROCESSREAED");
	Serial.print("Making reply with MTI ");
	Serial.println(reply.data[0], HEX);
	Serial.println(reply.data[1], HEX);
	//TODO presume datagram?
	Serial.print("length = ");
	Serial.println(length, DEC);
	Serial.print("address = ");
	Serial.println(address, HEX);
	Serial.print("space = ");
	Serial.println(space, HEX);
	Serial.print("data offset in datagram = ");
	Serial.println(datagram_offset);
	//And, now do something useful.
	//first check the space?
	switch(space)
	{
		case 0xFD: //Configuration space...basically just EEPROM
			reply.length = length+datagram_offset;
			for(uint8_t i = 0; i < length; ++i)
			{
				reply.data[datagram_offset+i] = eeprom_read_byte((uint8_t*)(address+i)); //TODO check that address is < 255!
			}
			if(sendDatagram(&reply))
			return DATAGRAM_ERROR_OK;
			//else
			//have them resend
			return DATAGRAM_REJECTED_BUFFER_FULL;
		
		case 0xFF: //CDI request.
			Serial.println("CDI request.");
			reply.length = readCDI(address, length, &(reply.data[datagram_offset])) + 6;
			Serial.print("total length of CDI reply = ");
			Serial.println(reply.length, DEC);
			if(sendDatagram(&reply))
			return DATAGRAM_ERROR_OK;
			//else
			//have them resend
			return DATAGRAM_REJECTED_BUFFER_FULL;
		//case 0xFE: //"All memory" access. Just give them what they want?
			//Serial.println("all memory request. ignoring");
		//	break;
	}
	//Serial.println("NAKing");
	return DATAGRAM_REJECTED; //send a NAK. Is this what we really want?
}

uint16_t DCC_Proxy::MACProcessWrite(void)
{
	uint32_t address = getAddress(_rxDatagramBuffer->data);
	uint8_t space = decodeSpace(_rxDatagramBuffer->data);
	uint8_t datagram_offset = (_rxDatagramBuffer->data[1]&0x03)?6:7; //if the space is encoded in the flags in data[1], then the payload begins at byte 6; else, the payload begins after the space byte, at byte 7.
	uint8_t length = (_rxDatagramBuffer->length)-datagram_offset;
	
	//Serial.println("Write Command");
	//Serial.print("length =");
	//Serial.println(length, HEX);
	//Serial.print("address =");
	//Serial.println(address, HEX);
	//Serial.print("space =");
	//Serial.println(space, HEX);
	
	//And, now do something useful.
	//first check the space?
	switch(space)
	{
		case 0xFD: //Configuration space, but basically just EEPROM
			for(uint8_t i = 0; i < length; ++i)
			{
				if( (address+i) < 0x0FF8)
				eeprom_write_byte((uint8_t *)(address+i), _rxDatagramBuffer->data[datagram_offset+i]);
			}
			return DATAGRAM_ERROR_OK;
		case 0xFF: //CDI request. We don't permit writes here
			//TODO
			break;
		case 0xFE: //"All memory" access. Just give them what they want?
			//TODO
			break;	
	}
	return DATAGRAM_REJECTED; //send a NAK. Is this what we really want?
}

uint16_t DCC_Proxy::MACProcessCommand(void)
{
	OLCB_Datagram reply;
	reply.data[0] = DATAGRAM_MEMCONFIG;
	memcpy(&(reply.destination), &(_rxDatagramBuffer->source), sizeof(OLCB_NodeID));
	switch (_rxDatagramBuffer->data[1]&0xFC)
	{
		case MAC_CMD_GET_CONFIG_OPTIONS:
			Serial.println("MAC_CMD_GET_CONFIG_OPTIONS");
			reply.length = 7;
			reply.data[1] = MAC_CMD_GET_CONFIG_OPTIONS_REPLY;
			reply.data[2] = MAC_CONFIG_OPTIONS_UNALIGNED_READS | MAC_CONFIG_OPTIONS_UNALIGNED_WRITES | MAC_CONFIG_OPTIONS_MFG_ACDI_FD_READ | MAC_CONFIG_OPTIONS_USR_ACDI_FC_READ; //available commands byte 1
			reply.data[3] = 0x00; //available commands byte 2
			reply.data[4] = MAC_CONFIG_OPTIONS_1_BYTE_WRITE | MAC_CONFIG_OPTIONS_2_BYTE_WRITE | MAC_CONFIG_OPTIONS_4_BYTE_WRITE | MAC_CONFIG_OPTIONS_64_BYTE_WRITE | MAC_CONFIG_OPTIONS_ARBITRARY_WRITE;
			reply.data[5] = 0xFF; //highest address space
			reply.data[6] = 0xFD; //lowest address space
			if(sendDatagram(&reply))
				return DATAGRAM_ERROR_OK;
			//else
			//have them resend
			return DATAGRAM_REJECTED_BUFFER_FULL;
		case MAC_CMD_GET_ADD_SPACE_INFO:
			Serial.println("MAC_CMD_GET_ADD_SPACE_INFO");
			reply.length = 8; //minimally!
			reply.data[1] = MAC_CMD_GET_ADD_SPACE_INFO_REPLY;
			reply.data[2] = _rxDatagramBuffer->data[2];
			reply.data[3] = 0x00; //largest address
			reply.data[4] = 0x00;
			reply.data[5] = 0x00;
			reply.data[6] = 0x00; //largest address continued
			reply.data[7] = 0x01; //flags; read-only be default
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
					//largest address is 0x3FF.
					reply.data[3] = 0x00;
					reply.data[4] = 0x00;
					reply.data[5] = 0x03;
					reply.data[6] = 0xFF;
					reply.data[7] = 0x00; //can write
					break;
			}
			if(sendDatagram(&reply))
				return DATAGRAM_ERROR_OK;
			//else
			return DATAGRAM_REJECTED_BUFFER_FULL;
		case MAC_CMD_RESETS:
			Serial.println("MAC_CMD_RESETS");
			// force restart (may not reply?)
			if ((_rxDatagramBuffer->data[1]&0x03) == 0x01)
			{ // restart/reboot?
				Serial.println("restart");
				//TODO tell other handlers that we need to write out anything that needs to be saved!!!
				//        cli();
				//        ((void (*)(void))0xF000)();
				soft_reset();
			}
			else if((_rxDatagramBuffer->data[1]&0x03) == 0x02) //factory reset!
			{
				Serial.println("factory reset");
				//check NID first!
				OLCB_NodeID n;
				memcpy(&n, &(_rxDatagramBuffer->data[2]), 6);
				if(n.sameNID(NID))
				{
					//_eventHandler->factoryReset();
					//cli();
					//((void (*)(void))0xF000)();
					soft_reset();
				}
			}
			// TODO: Handle other cases
			break;
		case MAC_CMD_LOCK:
			Serial.println("MAC_CMD_LOCK");
			break;
		case MAC_CMD_GET_UNIQUEID:
			Serial.println("MAC_CMD_GET_UNIQUEID");
			break; //not a P/C, don't care.
		case MAC_CMD_FREEZE:
			Serial.println("MAC_CMD_FREEZE");
			break;
		case MAC_CMD_INDICATE:
			Serial.println("MAC_CMD_INDICATE");
			break;
		default:
			Serial.println("Not recognized");
			break;
	}

	return DATAGRAM_REJECTED;
}

uint8_t DCC_Proxy::readCDI(uint16_t address, uint8_t length, uint8_t *data)
{
	//This method gets called by configuration handlers. We are being requested for the CDI file, a chunk at a time.
	//Serial.println("readCDI");
	//Serial.print("length: ");
	//Serial.println(length);
	uint16_t capacity = sizeof(cdixml)+1;
	if( (length+address) > (capacity) ) //too much! Would cause overflow
	//caculate a shorter length to prevent overflow
	length = capacity - address;
	//Serial.print("modified length: ");
	//Serial.println(length);
	//Serial.print("Read from index ");
	//Serial.println(address, DEC);
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
			if(c == '$') //major hardware version
			c = eeprom_read_byte((uint8_t *)0x0FF8)+48;
			else if(c == '%') //minor hardware version
			c = eeprom_read_byte((uint8_t *)0x0FF9)+48;
			*(data+(i)) = c;
		}
	}
	//Serial.println("===");
	//for(i = 0; i < length; ++i)
	//Serial.println(*(data+i), HEX);
	return length;
}


