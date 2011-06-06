#ifndef __LOCOMOTIVE_H__
#define __LOCOMOTIVE_H__

#include <OLCB_Datagram_Handler.h>
#include <OLCB_NodeID.h>

#define INITIAL 0
#define ATTACHING 1
#define ATTACHED 2


#define DATAGRAM_MOTIVE_ATTACH          1
#define DATAGRAM_MOTIVE_ATTACHED        2
#define DATAGRAM_MOTIVE_ATTACH_DENIED   3
#define DATAGRAM_MOTIVE_RELEASE         4
#define DATAGRAM_MOTIVE_RELEASED        5
#define DATAGRAM_MOTIVE_SET_SPEED       6
#define DATAGRAM_MOTIVE_SET_FUNCTION    7


class Locomotive : public OLCB_Datagram_Handler
{
 public:
  Locomotive() :available(true), state(INITIAL), speed(0), direction(1) {}

  void init(void);
  void update(void);
  bool processDatagram(void);
  void datagramResult(bool accepted, uint16_t errorcode);
  
  //used to determine if this locomotive can be safely deleted.
  bool isAvailable(void) { return available; }
 
  static DCCPacketScheduler *packetScheduler 
 private:
  //some methods for handling various possible incoming datagrams
  bool attachDatagram(void);
  bool releaseDatagram(void);
  bool setSpeedDatagram(void);
  bool setFunctionDatagram(void);
 
 
  uint8_t speed; //in percent full throttle
  int8_t direction;
  uint16_t functions; //stores function states
  OLCB_NodeID throttle;
  bool available;
  uint8_t state;
  uint16_t timer;
};

#endif