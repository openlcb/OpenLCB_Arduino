#ifndef __OLCB_CAN_LINK_H__
#define __OLCB_CAN_LINK_H__

#include <stdint.h>
#include <wprogram.h>

#include "OLCB_Link.h"
#include "OLCB_CAN_Buffer.h"
#include "OLCB_NodeID.h"
#include "OLCB_Event.h"
#include "OLCB_Datagram.h"
#include "OLCB_Stream.h"
#include "OLCB_AliasCache.h"

// state machine definitions
#define STATE_INITIAL 0
#define STATE_WAIT_CONFIRM 10
#define STATE_ALIAS_ASSIGNED 20
#define STATE_INITIALIZED 30

// time to wait between last CIM and RIM
#define CONFIRM_WAIT_TIME 500

class OLCB_CAN_Link : public OLCB_Link
{
 public:
  OLCB_CAN_Link(OLCB_NodeID *id):OLCB_Link(id), _aliasCacheTimer(-1), _aliasTimer(-1)
  {
//    Serial.print("OLCB_CAN_Link: ");
//    Serial.println((uint16_t)id, HEX);

    _translationCache.init(10); //initialize nida cache to 10 entries. Might be conservative
    _nodeIDToBeVerified.set(0,0,0,0,0,0);
  }
  
  bool initialize(void);
  
  bool negotiateAlias(OLCB_NodeID *nid);
  
  bool handleTransportLevel(void);
  virtual void update(void);
    
  bool sendEvent(OLCB_Event *event) {return false;}
  
  uint8_t sendDatagramFragment(OLCB_Datagram *datagram, uint8_t start);
  bool ackDatagram(OLCB_NodeID *source, OLCB_NodeID *dest);
  bool nakDatagram(OLCB_NodeID *source, OLCB_NodeID *dest, int reason);
  
  bool sendStream(OLCB_Stream *stream) {return false;}
  //Not sure that this is how streams should work at all!
  
  void sendVerifiedNID(OLCB_NodeID *nid);
  
  bool addVNode(OLCB_NodeID *NID)
  {
    return negotiateAlias(NID);//TODO This can sometimes fail!
  }
  
// protected:
  
// private:
  //OLCB_CAN_Buffer txBuffer, rxBuffer;
  OLCB_Buffer txBuffer, rxBuffer;

  /**
   * Send the next CIM message.  Return true it you can/did,
   * false if you didn't.
   */
  bool sendCIM(uint8_t i);
  
  /**
   * Send an RIM message.  Return true it you can/did,
   * false if you didn't.
   */
  bool sendRIM();
  
  /**
   * Send the InitializationComplete when everything is OK.
   *  Return true it you can/did,
   * false if you didn't.
   */
  bool sendInitializationComplete();
  
  /**
   * Get next possible alias value during CIM/RIM resolution.
   */
  void nextAlias();
  
  /** 
   * Restart allocation process at next alias
   */
  void restart();
  
  uint16_t getAlias();
 
  uint32_t _aliasTimer, _aliasCacheTimer; // used to wait for specific times (Arduino type definition)
  uint32_t lfsr1, lfsr2;  // PRNG sequence value: lfsr1 is upper 24 bits, lfsr2 lower
  uint8_t state; // internal state counter, starts at zero
  
  OLCB_NodeID _nodeIDToBeVerified;
  OLCB_NodeID *_NIDtoNegotiate;
  OLCB_AliasCache _translationCache;
  
  /*Methods for handling nida caching*/
  bool sendNIDVerifyRequest(OLCB_NodeID *nid);
  
  bool sendAMR(OLCB_NodeID *nid);
  
};

#endif //__OLCB_LINK_H__