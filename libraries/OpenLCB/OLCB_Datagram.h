#ifndef __OLCB_DATAGRAM_H__
#define __OLCB_DATAGRAM_H__

#include "OLCB_NodeID.h"


/*************************
 A few things to worry about.
 Sending a datagram:
   -> datagram chunk 1
   -> datagram chunk 2, etc.
   <- ACK
     or
   <- NAK
   Need a way to retrieve the ACK or NAK from the Link Controller. Means that certain kinds of events need to be passed to datagrams. Hrm.
*************************/

class OLCB_Datagram
{
 public:
  OLCB_Datagram() {return;}
  
  OLCB_NodeID* destination;
  uint8_t len;
  uint8_t *data;
 protected:
};

struct OLCB_DatagramFragment
{
  uint8_t data[10];
  uint8_t size;
};

#endif