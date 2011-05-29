#ifndef __OLCB_LINK_H__
#define __OLCB_LINK_H__

#include <can.h>
#include <stdint.h>
#include "OLCB_Event.h"
#include "OLCB_Handler.h"
#include "OLCB_Datagram.h"
#include "OLCB_Stream.h"
#include "OLCB_NodeID.h"


/* An abstract base class representing a physical link to the outside world */

class OLCB_Link
{
 public:
  OLCB_Link(OLCB_NodeID &id) : _nodeID(&id)
  {
    //create default handler
    _handlers = &_default_handler;
    _handlers->init();
  }
  
  virtual bool initialize(void) {return false;} //called once, returns true if init succeeded.
    
  virtual void update(void) {return;} //called repeatedly

  void addHandler(OLCB_Handler &handler)
  {
    handler.next = _handlers;
    _handlers = &handler;
  }
  
  virtual bool sendEvent(OLCB_Event &event) {return false;}
  
  virtual bool sendDatagram(OLCB_Datagram &datagram) {return false;}
  
  virtual bool sendStream(OLCB_Stream &stream) {return false;}
  //Not sure that this is how streams should work at all!
  
  
 protected:
  OLCB_Handler* _handlers;
  OLCB_Handler _default_handler;
  OLCB_NodeID* _nodeID;
};

#endif //__OLCB_LINK_H__