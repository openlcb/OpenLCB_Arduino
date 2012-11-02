// #ifdef DO_OLCB_DCC_TRAIN
#include <avr/eeprom.h>
#include <OpenLCB.h>
#include "DCC_Proxy.h"
#include "float16.h"
#include <reset.h>
#include <avr/eeprom.h>

#define PROXY_SLOT_OCCUPIED 01,01,00,00,00,00,03,05
#define PROXY_SLOT_FREE 01,01,00,00,00,00,03,04
#define COMMAND_STATION_EVENT 01,01,00,00,00,00,04,01

/***
Messages under a single MTI: Priority 1, index 15, addressed => MTI 0x05E8, CAN frame [195E8sss] fd dd
Content (after address bytes):
1st byte selects sub-instruction. High nibble selects protocol (Train Protocol; DCC; and future ones). Low nibble is a specific operation.
Train Protocol:
0x00 Speed instruction, followed by 2 bytes float16 speed (sign is direction)
0x01 Function instruction: 3 byte address, 2 bytes content
To allow read back, the results of these operations are stored in the 0xFE memory space of the node. The train can also be controlled by direct write of these via the memory protocol.
***/

void DCC_Proxy::initialize(void)
{
  _dcc_address = 0;
  _dcc_address_kind = 0;
  _timer = millis();
  _speed = 0;
  _dirty_speed = _dirty_FX = 0;
  _FX = 0;
  _active = 0;
  _producer_identified_flag = true;
  _speed_steps = 0;
  _initial_blast = 10;
  _update_alias = 0;
}

void DCC_Proxy::create(OLCB_Link *link, OLCB_NodeID *nid, uint8_t *address)
{
  //we begin by reading some state information from EEPROM.
  _eeprom_address = address;
  _dcc_address = (eeprom_read_byte(_eeprom_address+96) << 8) | eeprom_read_byte(_eeprom_address+97);
  Serial.println(_dcc_address);
  _dcc_address_kind = eeprom_read_byte(_eeprom_address+98);
  Serial.println(_dcc_address_kind);
  //detemine what kind of address this is
  if(_dcc_address_kind) //if long address
  {
    _dcc_address |= 0xC000; //make sure top two bits are 1, in conformance with RP 9.2.1
  }
  _speed_steps = eeprom_read_byte(_eeprom_address+99);
  Serial.println(_speed_steps);

  //create a NID TODO THIS SHOULD BE MANUFACTURER ASSIGNED! READ FROM EEPROM!
  nid->val[0] = 6;
  nid->val[1] = 1;
  nid->val[2] = 0;
  nid->val[3] = 0;
  nid->val[4] = _dcc_address >> 8;
  nid->val[5] = _dcc_address & 0xFF;

  //and copy this to the cs address
  DCC_NodeID.val[0] = 6;
  DCC_NodeID.val[1] = 1;
  DCC_NodeID.val[2] = 0;
  DCC_NodeID.val[3] = 1;
  DCC_NodeID.val[4] = nid->val[4];
  DCC_NodeID.val[5] = nid->val[5];

  OLCB_Datagram_Handler::create(link, nid);
}

void DCC_Proxy::update(void)
{
  if(!isPermitted())
    return;

  //check to see if we need to emit a producer identified event
  if (_producer_identified_flag)
  {
    OLCB_Event e(PROXY_SLOT_OCCUPIED);
    while (!_link->sendProducerIdentified(NID, &e));
    _producer_identified_flag = false;
  }

  uint32_t new_time = millis();
  if(_active)
  {
    if(_update_alias)
    {
      Serial.println("reacquiring alias");
      _update_alias = !_link->sendVerifyNID(NID, &DCC_NodeID);
      if(!_update_alias) //message has gone through
        _initial_blast = 10; //refresh status
      return;
    }

    //see if we need to send out any periodic updates
    if(_initial_blast)
    {
      Serial.print("Initial blast!! TRYING AGAIN! ");
      Serial.println(_initial_blast);
      _initial_blast--;
      //this is a hack to make sure the first commands get through while DCS Vnode is being allocated
      _dirty_speed = _dirty_FX = true; //send again!
    }

    if((new_time - _timer) >= 60000)
    {
      _timer = new_time;
      _dirty_speed = true;
      _dirty_FX = true;
    }
    if( _dirty_speed ) //if we are connected to CS, and either need to send an update or one minute has passed since last update:
    {
      sendSpeed();
    }
    if( _dirty_FX ) //if we are connected to CS, and either need to send an update or one minute has passed since last update:
    {
      sendFX();
    }
  }
  OLCB_Datagram_Handler::update();
}

void DCC_Proxy::sendSpeed()
{
  if(!_active)
  {
    _dirty_speed = false;
    return;
  }
  Serial.print("Loco at address ");
  Serial.print(_dcc_address, DEC);
  Serial.print(" speed update to ");
  Serial.println(_speed);
  OLCB_Datagram d;
  memcpy(&(d.destination), &DCC_NodeID, sizeof(OLCB_NodeID));
  d.data[0] = DATAGRAM_MEMCONFIG;
  d.data[1] = MAC_CMD_WRITE | 0x02; //flags for All Memory space
  d.data[2] = 0x00; //address byte 0
  d.data[3] = 0x00; //address byte 1
  d.data[4] = 0x00; //address byte 2
  d.data[5] = 0x00; //address byte 3
  d.data[6] = _speed;
  d.data[7] = _speed_steps; //we use 128 speed steps exclusively here. This should be a configuration option
  d.length = 8;
  _dirty_speed = !sendDatagram(&d); //try again if transmission fails...probably not a good idea!!!
}

void DCC_Proxy::sendFX()
{
  if(!_active)
  {
    _dirty_FX = false;
    return;
  }
  Serial.print("Loco at address ");
  Serial.print(_dcc_address, DEC);
  Serial.print(" FX update to ");
  Serial.println(_FX, HEX);
  OLCB_Datagram d;
  memcpy(&(d.destination), &DCC_NodeID, sizeof(OLCB_NodeID));
  d.data[0] = DATAGRAM_MEMCONFIG;
  d.data[1] = MAC_CMD_WRITE | 0x02; //flags for All Memory space
  d.data[2] = 0x00; //address byte 0
  d.data[3] = 0x00; //address byte 1
  d.data[4] = 0x00; //address byte 2
  d.data[5] = 0x01; //address byte 3
  d.data[6] = _FX >> 8;
  d.data[7] = _FX & 0xFF;
  d.length = 8;
  _dirty_FX = !sendDatagram(&d); //try again if transmission fails...probably not a good idea!!!
}

void DCC_Proxy::datagramResult(bool accepted, uint16_t errorcode)
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

  if(accepted)
    _initial_blast = 0; //it went through, that is enough.
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

uint16_t DCC_Proxy::processDatagram(void)
{
  Serial.println("Got a datagram");

  if(!isPermitted()) // && (_rxDatagramBuffer->destination == *OLCB_Virtual_Node::NID))
    return DATAGRAM_REJECTED; // TODO NEED A WYA TO HANDLE THE UNPERMITTED CASE


  //make sure that it is a train control datagram, and handle that accordingly
  if(!_rxDatagramBuffer->length) //check for zeo length
      return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;

  switch(_rxDatagramBuffer->data[0])
  {
  case DATAGRAM_MOTIVE:
    return handleTractionDatagram(_rxDatagramBuffer);
  case DATAGRAM_MEMCONFIG:
    return handleMemConfigDatagram(_rxDatagramBuffer);
  }		
  Serial.println("not a datagram we know"); //TODO HANDLE CONFIGURATION DATAGRAMS!!
  return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;
}

uint16_t DCC_Proxy::handleTractionDatagram(OLCB_Datagram *datagram)
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
    return handleSetSpeedDatagram(datagram);
  case DATAGRAM_MOTIVE_GETSPEED:
    Serial.println("getspeed");
    return handleGetSpeedDatagram(datagram);
  case DATAGRAM_MOTIVE_ESTOP:
    Serial.println("estop");
    return handleEStopDatagram();
    //case DATAGRAM_MOTIVE_SETFX:
    //return handleSetFXDatagram(datagram);
    //case DATAGRAM_MOTIVE_GETFX:
    //return handleGetFXDatagram(datagram);
  }
  return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;
}

uint16_t DCC_Proxy::handleEStopDatagram(void)
{
  //if(DCC_Proxy_isAttached(&(datagram->source)))
  //{
  //incoming speed is a signed float16
  //we store it as a signed 8-bit int, with -1/1 = stop (and 0 = estop), and -127/127 = max speed
  //notice that it is not enough to get the raw integral value of the float16, but we must scale it, because the DCC speed steps != abslute speed, but throttle notches. So we have to account for what motor speed each notch represents, and choose the appropriate notch..yuck!
    //notice also the significance of -0 in the input to indicate stopped, reverse facing.
  Serial.println("Speed change");
  _speed = 0; //estop
  _active = true; //assume it has been placed on layout.
  _dirty_speed = true; //force transmission to CS at next update.
  return DATAGRAM_ERROR_OK;
}

uint16_t DCC_Proxy::handleSetSpeedDatagram(OLCB_Datagram *datagram)
{
  //if(DCC_Proxy_isAttached(&(datagram->source)))
  //{
  //incoming speed is a signed float16
  //we store it as a signed 8-bit int, with -1/1 = stop (and 0 = estop), and -127/127 = max speed
  //notice that it is not enough to get the raw integral value of the float16, but we must scale it, because the DCC speed steps != abslute speed, but throttle notches. So we have to account for what motor speed each notch represents, and choose the appropriate notch..yuck!
    //notice also the significance of -0 in the input to indicate stopped, reverse facing.
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
  if(new_speed == 0)
  {
    if(f_val.bits & 0x8000) //-0
        _speed = -1;
    else
      _speed = 1;
  }
  else
  {
    _speed = map(new_speed, -100, 100, -126, 126);
    if(_speed < 0) --_speed; //to get it in range -2..-126
    else ++_speed; //to get it in range 2..126
  }
  _active = true; //assume it has been placed on layout.
  _dirty_speed = true; //force transmission to CS at next update.
  return DATAGRAM_ERROR_OK;
}

uint16_t DCC_Proxy::handleGetSpeedDatagram(OLCB_Datagram *datagram)
{
  OLCB_Datagram reply;
  memcpy(&(reply.destination), &(_rxDatagramBuffer->source), sizeof(OLCB_NodeID));
  reply.data[0] = DATAGRAM_MOTIVE;
  reply.data[1] = DATAGRAM_MOTIVE_GETSPEED;
  float f_speed;

  if(_active) //if active, send ucrrent speed, else send 0;
  {
    if(_speed == -1)
      f_speed = -0;
    else if(_speed == 1)
      f_speed = 0;
    else if(_speed < 0)
      f_speed = _speed + 1; //get it from -2..-127 to -1..-126
    else
      f_speed = _speed -1; //from 2..127 to 1..126
  }
  else
  {
    f_speed = 0;
  }
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

uint16_t DCC_Proxy::handleMemConfigDatagram(OLCB_Datagram* datagram)
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
  uint8_t datagram_offset = (_rxDatagramBuffer->data[1]&0x03)?6:7; //if the space is encoded in the flags in data[1], then the payload begins at byte 6; else, the payload begins after the space byte,at byte 7.
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
      reply.data[datagram_offset+i] = eeprom_read_byte((uint8_t*)(address+i+_eeprom_address)); //TODO check that address is < 255!
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
  uint8_t datagram_offset = (_rxDatagramBuffer->data[1]&0x03)?6:7; //if the space is encoded in the flags in data[1], then the payload begins at byte 6; else, the payload begins after the space byte,at byte 7.
    uint8_t length = (_rxDatagramBuffer->length)-datagram_offset;

  Serial.println("Write Command");
  Serial.print("length =");
  Serial.println(length, HEX);
  Serial.print("address =");
  Serial.println(address, HEX);
  Serial.print("space =");
  Serial.println(space, HEX);

  //And, now do something useful.
  //first check the space?
  switch(space)
  {
  case 0xFD: //Configuration space, but basically just EEPROM
    for(uint8_t i = 0; i < length; ++i)
    {
      if( (address+i) < 0x0FF8)
        eeprom_write_byte((uint8_t *)(address+i+_eeprom_address), _rxDatagramBuffer->data[datagram_offset+i]);
    }
    return DATAGRAM_ERROR_OK;
  case 0xFF: //CDI request. We don't permit writes here
    //TODO
    break;
  case 0xFE: //"All memory" access. Just give them what they want?
    //TODO
    break;	
  case 0xF9: //DCC function space
    Serial.print("DCC Function ");
    Serial.print(address, DEC);
    if(_rxDatagramBuffer->data[datagram_offset])
    {
      Serial.println(" on");
      _FX |= (1<<address);
    }
    else
    {
      _FX &= ~(1<<address);
      Serial.println(" off");
    }
    _active = true;
    _dirty_FX = true;
    return DATAGRAM_ERROR_OK;
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
      //soft_reset(); don't actually do a reset, just reload the stuff from eeprom
      //reason is that we don't want to disturb other vnodes on this piece of hardware.
      create(_link, NID, _eeprom_address);
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
        //TODO reset address, etc.
        create(_link, NID, _eeprom_address);
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

bool DCC_Proxy::handleMessage(OLCB_Buffer *buffer)
{
  if(buffer->isTractionControl())
  {
    Serial.println("Traction!");
    //do stuff here.
    switch(buffer->data[2])
    {
      case 0x00: //speed
      {
        _float16_shape_type f_val;
        Serial.println("New MTI Speed change");
        Serial.println("raw data");
        f_val.words.msw = buffer->data[3];
        f_val.words.lsw = buffer->data[4];
        Serial.print(buffer->data[3], HEX);
        Serial.print(" ");
        Serial.println(buffer->data[4], HEX);
        float new_speed = float16_to_float32(f_val);
        Serial.println(new_speed);
        Serial.println("----");
        if(new_speed == 0)
        {
          if(f_val.bits & 0x8000) //-0
            _speed = -1;
          else
            _speed = 1;
        }
        else
        {
          _speed = map(new_speed, -100, 100, -126, 126);
          if(_speed < 0) --_speed; //to get it in range -2..-126
          else ++_speed; //to get it in range 2..126
        }
      _active = true; //assume it has been placed on layout.
      _dirty_speed = true; //force transmission to CS at next update.
      }
        break;
      case 0x01: //function
      {
        uint32_t address;
        address = (buffer->data[3] << 12) | (buffer->data[4] << 8) | (buffer->data[5]);
        if(address >= 32)
          break; //can't handle it, just ditch it.
        if(buffer->data[6] | buffer->data[7]) //treat non-zero as "on", zero as "off"
          _FX |= (1<<address);
        else
          _FX &= ~(1<<address);
        _active = true;
        _dirty_FX = true;
      }
        break;
    }
  }
  else if (buffer->isIdentifyProducers())
  {
    //get the EventID, see if it matches the preset one.
    OLCB_Event evt;
    buffer->getEventID(&evt);
    if (evt.is(PROXY_SLOT_OCCUPIED)) //send an event produced TODO only occupied if it really is!
    {
      _producer_identified_flag = true;
      return true;
    }
  }
  else if(buffer->isProducerIdentified())
  {
    OLCB_Event evt;
    buffer->getEventID(&evt);
    if(evt.is(COMMAND_STATION_EVENT))
    {
      //invalidate our alias
      _update_alias = 1;
    }
  }
  else return OLCB_Datagram_Handler::handleMessage(buffer);
}


