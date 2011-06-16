#ifndef __LOCOMOTIVE_FACTORY_H__
#define __LOCOMOTIVE_FACTORY_H__

#include <OLCB_Datagram_Handler.h>
#include "Locomotive.h"

#if defined(__AVR_AT90CAN128__) || defined(__AVR_ATMEGA1280__)
#define NUM_SLOTS 25
#else
#define NUM_SLOTS 5
#endif

class LocomotiveFactory : public OLCB_Datagram_Handler
{
  public:
  
  bool verifyNID(OLCB_NodeID *nid);
  
  void update(void);
  
 private:
  Locomotive _locos[NUM_SLOTS];
};

#endif
