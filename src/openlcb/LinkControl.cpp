// makes this an Arduino file
#include "WConstants.h"

#include "LinkControl.h"

#include "OpenLcbCanBuffer.h"
#include "OpenLcbCanInterface.h"
#include "NodeID.h"

// state machine definitions
#define STATE_INITIAL 0
#define STATE_WAIT_CONFIRM 10
#define STATE_ALIAS_ASSIGNED 20
#define STATE_INITIALIZED 30

// time to wait between last CIM and RIM
#define CONFIRM_WAIT_TIME 500

#include "logging.h"

LinkControl::LinkControl(OpenLcbCanBuffer* b, NodeID* n) {
  txBuffer = b;
  nid = n;
  // initialize sequence from node ID
  lfsr1 = (nid->val[0] << 16) | (nid->val[1] << 8) | (nid->val[2]);
  lfsr2 = (nid->val[3] << 16) | (nid->val[4] << 8) | (nid->val[5]);
  // set up for next (first) alias
  reset();
}

void LinkControl::nextAlias() {
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

void LinkControl::reset() {
  state = STATE_INITIAL;
  // take the 1st from the sequence
  nextAlias();
  log("new key ");loghex(lfsr1);loghex(lfsr2);log(" alias ");loghex(getAlias());logln();
}

bool LinkControl::sendCIM(int i) {
  if (!OpenLcb_can_xmt_ready(txBuffer)) return false;  // couldn't send just now  if (!isTxBufferFree()) return false;  // couldn't send just now
  txBuffer->setCIM(i,nid->val[i],getAlias());
  OpenLcb_can_queue_xmt_wait(txBuffer);  // wait for queue, but earlier check says will succeed
  return true;
}

bool LinkControl::sendRIM() {
  if (!OpenLcb_can_xmt_ready(txBuffer)) return false;  // couldn't send just now  if (!isTxBufferFree()) return false;  // couldn't send just now
  txBuffer->setRIM(getAlias());
  OpenLcb_can_queue_xmt_wait(txBuffer);  // wait for queue, but earlier check says will succeed
  return true;
}

bool LinkControl::sendInitializationComplete() {
  if (!OpenLcb_can_xmt_ready(txBuffer)) return false;  // couldn't send just now  if (!isTxBufferFree()) return false;  // couldn't send just now
  txBuffer->setInitializationComplete(getAlias(), nid);
  OpenLcb_can_queue_xmt_wait(txBuffer);  // wait for queue, but earlier check says will succeed
  return true;
}

void LinkControl::check() {
  // find current state and act
  if (state == STATE_INITIALIZED) return;
  switch (state) {
  case STATE_INITIAL+0:
  case STATE_INITIAL+1:
  case STATE_INITIAL+2:
  case STATE_INITIAL+3:
  case STATE_INITIAL+4:
  case STATE_INITIAL+5:
    // send next CIM message if possible
    if (sendCIM(state-STATE_INITIAL)) 
      state++;
    return;
  case STATE_INITIAL+6:
    // last CIM, sent, wait for delay
    timer = millis();
    state = STATE_WAIT_CONFIRM; 
    log("alias assigned ");loghex(getAlias());logln();

    return;
  case STATE_WAIT_CONFIRM:
    if ( (millis() > timer+CONFIRM_WAIT_TIME) && sendRIM()) {
      state = STATE_ALIAS_ASSIGNED;
    }
    return;
  case STATE_ALIAS_ASSIGNED:
    // send init
    if (sendInitializationComplete()) 
      state = STATE_INITIALIZED;
    return;
  default:
    return;
  }
}

bool LinkControl::linkInitialized() {
  return state == STATE_INITIALIZED;
}

unsigned int LinkControl::getAlias() {
  return (lfsr1 ^ lfsr2 ^ (lfsr1>>12) ^ (lfsr2>>12) )&0xFFF;
}

void LinkControl::receivedFrame(OpenLcbCanBuffer* rcv) {
   // check received message
   // see if this is a frame with our alias
   if (getAlias() == rcv->getSourceAlias()) {
     // somebody else trying to use this one, see to what extent
     if (rcv->isCIM()) {
       // somebody else trying to allocate, tell them
       while (!sendRIM()) {}  // insist on sending it now.
     } else if (rcv->isRIM()) {
       // RIM frame is an error, restart
       reset();
     } else {
       // some other frame; this is an error! Restart
       reset();
     }
   }
   // see if this is a Verify request to us; first check type
   if (rcv->isVerifyNID()) {
     // check address
     NodeID n;
     rcv->getNodeID(&n);
     if (n.equals(nid)) {
       // reply; should be threaded, but isn't
       txBuffer->setVerifiedNID(nid);
       OpenLcb_can_queue_xmt_wait(txBuffer);
     }
   }
}
