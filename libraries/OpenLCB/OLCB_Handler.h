#ifndef __OLCB_HANDLER_H__
#define __OLCB_HANDLER_H__

#include "OLCB_NodeID.h"
#include "OLCB_CAN_Buffer.h" ///SHOULD BE OLCB_Buffer.h! TODO!

class OLCB_Handler
{
 public:
  OLCB_Handler() : next(0) {return;}
  virtual void init(void) {return;}
  void setNID(uint8_t b0, uint8_t b1, uint8_t b2, 
         uint8_t b3, uint8_t b4, uint8_t b5) {
      //Notice that using this method does not set an alias, if aliases are being used by your favored transport.
      _nodeID.set(b0,b1,b2,b3,b4,b5);
  }
  void setNID(OLCB_NodeID &NID)
  {
    memcpy(&_nodeID, &NID, sizeof(OLCB_NodeID));
  }
  void getNID(OLCB_NodeID &NID)
  {
    memcpy(&NID, &_nodeID, sizeof(OLCB_NodeID));
  }
  virtual void update(void) {return;}
  virtual bool handleFrame(OLCB_Buffer &buffer) {return false;}  
  virtual bool checkvNID(OLCB_NodeID &nid) {return false;}
  
  OLCB_Handler *next;
 protected:
  OLCB_NodeID _nodeID;
};

#endif