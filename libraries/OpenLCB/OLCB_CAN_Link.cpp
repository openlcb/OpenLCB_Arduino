#include <WProgram.h>
//Code taken from LinkControl.cpp

#include "OLCB_CAN_Link.h"


//////Initialization routine
// Based on check(), etc
bool OLCB_CAN_Link::initialize(void)
{
	if(!can_init(BITRATE_125_KBPS))
	{
	  Serial.println("Couldn't can_init()");
		return false;
	}
	
	//Negotiate the alise for this Node's real ID.
	if(!negotiateAlias(_nodeID))
	{
	  Serial.println("Couldn't initiate alias negotiation!");
	  return false;
	}
	while(!negotiateAlias(0)) //TODO THis isn't right at all.
	{
	  //check to see if any new messages require handling
    if(can_get_message(&rxBuffer))
    {
  	  handleTransportLevel();
  	}
  }
  return true;
}

bool OLCB_CAN_Link::negotiateAlias(OLCB_NodeID *nid=0)
{
  if(nid != NULL) //this is a request to start working on this NID's alias
  {
    if(_NIDtoNegotiate != NULL) //but we're busy with another one just now
      return false;
      
    //else
    // initialize sequence from node ID
    lfsr1 = (((uint32_t)nid->val[0]) << 16) | (((uint32_t)nid->val[1]) << 8) | ((uint32_t)nid->val[2]);
    lfsr2 = (((uint32_t)nid->val[3]) << 16) | (((uint32_t)nid->val[4]) << 8) | ((uint32_t)nid->val[5]);
    _NIDtoNegotiate = nid;
    return true;
  }

  //otherwise this is a request to continue working on the current NID
  if(state == STATE_INITIALIZED) //nothing to work on
    return true;
    

  Serial.print("state = ");
  Serial.println(state,HEX);
	switch (state) {
		case STATE_INITIAL+0:
		case STATE_INITIAL+1:
		case STATE_INITIAL+2:
		case STATE_INITIAL+3:
			// send next CIM message if possible
			if (sendCIM(state-STATE_INITIAL)) 
				state++;
			break;
		case STATE_INITIAL+4:
			// last CIM, sent, wait for delay
			_aliasTimer = millis();
			state = STATE_WAIT_CONFIRM; 
			break;
		case STATE_WAIT_CONFIRM:
			if ( (millis() > _aliasTimer+CONFIRM_WAIT_TIME) && sendRIM()) {
				state = STATE_ALIAS_ASSIGNED;
			}
			break;
		case STATE_ALIAS_ASSIGNED:
			// send init
			if (sendInitializationComplete()) {
				state = STATE_INITIALIZED;
				_NIDtoNegotiate = NULL;
				_aliasTimer = -1;
				return true;
			}
			break;
	}
	return false;
}

// send the next CIM message.  "i" is the 0-3 ordinal number of the message, which
// becomes F-C in the CIM itself. Returns true if successfully sent.
bool OLCB_CAN_Link::sendCIM(uint8_t i) {
  if (!can_check_free_buffer()) return false;  // couldn't send just now
  uint16_t fragment;
  switch (i) {
    case 0:  fragment = ( (_NIDtoNegotiate->val[0]<<4)&0xFF0) | ( (_NIDtoNegotiate->val[1] >> 4) &0xF);
             break;
    case 1:  fragment = ( (_NIDtoNegotiate->val[1]<<8)&0xF00) | ( _NIDtoNegotiate->val[2] &0xF);
             break;
    case 2:  fragment = ( (_NIDtoNegotiate->val[3]<<4)&0xFF0) | ( (_NIDtoNegotiate->val[4] >> 4) &0xF);
             break;
    default:
    case 3:  fragment = ( (_NIDtoNegotiate->val[4]<<8)&0xF00) | ( _NIDtoNegotiate->val[5] &0xF);
             break;
  }
  txBuffer.setCIM(i,fragment,getAlias());
  Serial.println("Sending CIM");
  while(!can_send_message(&txBuffer));  // wait for queue, but earlier check says will succeed
  return true;
}

bool OLCB_CAN_Link::sendRIM() {
  if (!can_check_free_buffer()) return false;  // couldn't send just now  if (!isTxBufferFree()) return false;  // couldn't send just now
  txBuffer.setRIM(getAlias());
  Serial.println("Sending RIM");
  while(!can_send_message(&txBuffer));  // wait for queue, but earlier check says will succeed
  return true;
}

bool OLCB_CAN_Link::sendInitializationComplete() {
  if (!can_check_free_buffer()) return false;  // couldn't send just now  if (!isTxBufferFree()) return false;  // couldn't send just now
  txBuffer.setInitializationComplete(getAlias(), _NIDtoNegotiate);
  while(!can_send_message(&txBuffer));    // wait for queue, but earlier check says will succeed
  return true;
}

void OLCB_CAN_Link::restart() {
  state = STATE_INITIAL;
  // take the 1st from the sequence
  nextAlias();
}

uint16_t OLCB_CAN_Link::getAlias() {
  return (lfsr1 ^ lfsr2 ^ (lfsr1>>12) ^ (lfsr2>>12) )&0xFFF;
}

void OLCB_CAN_Link::nextAlias() {
   // step the PRNG
   // First, form 2^9*val
   uint32_t temp1 = ((lfsr1<<9) | ((lfsr2>>15)&0x1FF)) & 0xFFFFFF;
   uint32_t temp2 = (lfsr2<<9) & 0xFFFFFF;
   
   // add
   lfsr2 = lfsr2 + temp2 + 0x7A4BA9l;
   lfsr1 = lfsr1 + temp1 + 0x1B0CA3l;
   
   // carry
   lfsr1 = (lfsr1 & 0xFFFFFF) | ((lfsr2&0xFF000000) >> 24);
   lfsr2 = lfsr2 & 0xFFFFFF;
}


/////// Methods for sending and receiving OLCB packets over CAN

bool OLCB_CAN_Link::handleTransportLevel()
{
    // see if this is a frame with our alias
    if (getAlias() == rxBuffer.getSourceAlias()) {
      Serial.println("Someone's using our alias!");
      // somebody else trying to use this one, see to what extent
      if (rxBuffer.isCIM()) {
            Serial.println("Someone's sent a CIM!");
        // somebody else trying to allocate, tell them
        while (!sendRIM()) {}  // insist on sending it now.
      } else if (rxBuffer.isRIM()) {
        // RIM frame is an error, restart
                    Serial.println("Someone's sent a RIM!");
        restart();
      } else {
        // some other frame; this is an error! Restart
        restart();
      }
      return true;
    }
    // see if this is a Verify request to us; first check type
    if (rxBuffer.isVerifyNID()) {
      // check address
      Serial.println("It's a verify ID!");
      OLCB_NodeID n;
      rxBuffer.getNodeID(&n);
      if (n == *_nodeID) //if the verification matches our nodeID, respond
      {
        // reply; should be threaded, but isn't
        txBuffer.setVerifiedNID(_nodeID);
        while(!can_send_message(&txBuffer));
      }
      else //it might yet match a virtual NID; check each handler to see
      {
        bool flag = false;
        //check all handlers, not just datagram handlers
        OLCB_Handler *iter = _handlers;
        while(iter != NULL && !flag)
        {
          if(iter->checkvNID(n))
          {
            txBuffer.setVerifiedNID(&n);
            while(!can_send_message(&txBuffer));
            flag = true;
          }
        }
      }
      return true;
    }
    else if (rxBuffer.isVerifyNIDglobal()) {
      // reply to global request
      // ToDo: This should be threaded
      txBuffer.setVerifiedNID(_nodeID);
      while(!can_send_message(&txBuffer));
      return true;
    }
    else if (rxBuffer.isVerifiedNID()) {
    // We have a packet that contains a verified NID. We might have requested this. Let's check.
      OLCB_NodeID n;
      rxBuffer.getNodeID(&n);
      Serial.println("Received a Verified Packet");
      Serial.println(n.alias, DEC);
      Serial.println("-----");
      Serial.println(n.val[0], HEX);
      Serial.println(n.val[1], HEX);
      Serial.println(n.val[2], HEX);
      Serial.println(n.val[3], HEX);
      Serial.println(n.val[4], HEX);
      Serial.println(n.val[5], HEX);
      Serial.println("=======");
      if(n == _nodeIDToBeVerified)
      {
        Serial.println("Caching it");
        //add it to the NID/NIDa translation cache
        _translationCache.add(n);
        //remove from temp buffer
        _nodeIDToBeVerified.set(0,0,0,0,0,0);
      }
      else
      {
        Serial.println("Not going to cache it");
        Serial.println(_nodeIDToBeVerified.val[0], HEX);
        Serial.println(_nodeIDToBeVerified.val[1], HEX);
        Serial.println(_nodeIDToBeVerified.val[2], HEX);
        Serial.println(_nodeIDToBeVerified.val[3], HEX);
        Serial.println(_nodeIDToBeVerified.val[4], HEX);
        Serial.println(_nodeIDToBeVerified.val[5], HEX);
        Serial.println("=======");
      }
      return true;
    }
    return false;
}

void OLCB_CAN_Link::update(void)
{
  //check to see if any new messages require handling
  if(can_get_message(&rxBuffer))
  {
    Serial.println("Got a message!");
    // See if this message is a CAN-level message that we should be handling.
    if(handleTransportLevel())
      return; //bail early, the packet grabbed isn't for any of the attached handlers to deal with.
    //let's see if we can make some hay of this to pass on to our handlers
    
    Serial.println("Not a message for Link to handle, should be passed on");
    OLCB_Handler *iter = _handlers;
    while(iter)
    {
      if(iter->handleFrame(rxBuffer))
      {
        Serial.println("Frame was handled successfully.");
        break;
      }
      iter = iter->next;
    }
  }
}


bool OLCB_CAN_Link::sendDatagram(OLCB_Datagram &datagram)
{
  //First things first: Do we know the nida of the destination?
  uint16_t nida = datagram.destination->alias;
  if(!nida)
  {
    //try the cache
    if(!_translationCache.getAliasByNID(*(datagram.destination))) //not cached!
    {
      //need to ask
      sendNIDVerifyRequest(*(datagram.destination)); //if it can't go through, it'll get called again. no need to loop.
      return false;
    }
  }
  
  //unpack the data into one or more CAN packets.
  Serial.println("Sending datagram with contents:");
  for(int i = 0; i < datagram.len; ++i)
    Serial.println(datagram.data[i], HEX);
  Serial.println("===========");
}

bool OLCB_CAN_Link::sendNIDVerifyRequest(OLCB_NodeID &nid)
{
  Serial.println("In sendNIDVerifyRequest.");
  //first, see if a request is pending
  if(!_nodeIDToBeVerified.empty() && (millis() - _aliasCacheTimer < 1000) ) //it's not zeros, and it hasn't yet been a full second since the last request
  {
    Serial.println("Previous request still outstanding");
    Serial.println(_nodeIDToBeVerified.val[0], HEX);
        Serial.println(_nodeIDToBeVerified.val[1], HEX);
        Serial.println(_nodeIDToBeVerified.val[2], HEX);
        Serial.println(_nodeIDToBeVerified.val[3], HEX);
        Serial.println(_nodeIDToBeVerified.val[4], HEX);
        Serial.println(_nodeIDToBeVerified.val[5], HEX);
        Serial.println("=======");
    Serial.println("Leaving sendNIDVerifyRequest");
    return false; //this seems a mistake. What if we never get a reply? Maybe we should let replies timeout?
    }
  if (!can_check_free_buffer()){
    Serial.println("No free buffer to xmit!");
      Serial.println("Leaving sendNIDVerifyRequest");
    return false;  // couldn't send just now
  }
  memcpy(&_nodeIDToBeVerified, &nid, sizeof(OLCB_NodeID));
  txBuffer.setVerifyNID(&nid);
  Serial.println("Queuing Message");
  while(!can_send_message(&txBuffer));  // wait for queue, but earlier check says will succeed
  Serial.println("Message sent!");
  _aliasCacheTimer = millis(); //set the clock going. A request will only be permitted to stand for 1 second.
  Serial.println("Leaving sendNIDVerifyRequest");
  return true;
}