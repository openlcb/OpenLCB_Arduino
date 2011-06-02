#ifndef __OLCB_HANDLER_H__
#define __OLCB_HANDLER_H__

#include "OLCB_NodeID.h"
#include "OLCB_Link.h"
#include "OLCB_CAN_Buffer.h" ///SHOULD BE OLCB_Buffer.h! TODO!

class OLCB_Link;

class OLCB_Handler
{
 public:
  OLCB_Handler() : NID(0), _link(0), next(0) {return;}

  virtual void init(void) {return;}

  virtual void setNID(OLCB_NodeID *newNID);  
  virtual void setLink(OLCB_Link *newLink);

  virtual void update(void) {return;}
  virtual bool handleFrame(OLCB_Buffer *buffer) {return false;}  
  virtual bool verifyNID(OLCB_NodeID *nid) {return false;}
  
  OLCB_Handler *next;  
  OLCB_NodeID *NID;
  
 protected:
  OLCB_Link *_link;
};

//class OLCB_VirtualNode : protected OLCP_Handler
//{
//}

#endif