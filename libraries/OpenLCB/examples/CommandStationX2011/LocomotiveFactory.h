#ifndef __LOCOMOTIVE_FACTORY_H__
#define __LOCOMOTIVE_FACTORY_H__

#include <OLCB_Datagram_Handler.h>
#include "Locomotive.h"

class LocomotiveFactory : public OLCB_Datagram_Handler
{
  public:
  
  bool verifyNID(OLCB_NodeID *nid);
  
  void update(void);
  
 private:
  Locomotive _locos[50];
};

#endif
