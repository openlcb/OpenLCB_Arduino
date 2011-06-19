#ifndef __THROTTLE_H__
#define __THROTTLE_H__

#include "OLCB_Datagram_Handler.h"

#define IDLE 0
#define ATTACHING 1
#define SETTING_SPEED 2
#define SETTING_FUNCTION 3
#define RELEASING 4

//datagram byte 0 (command identifier)le
#define DATAGRAM_MOTIVE                 0x30
//datagram byte 1 (sub-command identifier, lower nibble only; upper nibble reserverd)
#define DATAGRAM_MOTIVE_ATTACH          0x01
#define DATAGRAM_MOTIVE_ATTACHED        0x02
#define DATAGRAM_MOTIVE_ATTACH_DENIED   0x03
#define DATAGRAM_MOTIVE_RELEASE         0x04
#define DATAGRAM_MOTIVE_RELEASED        0x05
#define DATAGRAM_MOTIVE_SETSPEED        0x06
#define DATAGRAM_MOTIVE_GETSPEED        0x07
#define DATAGRAM_MOTIVE_SETFUNCTION     0x08
#define DATAGRAM_MOTIVE_GETFUNCTION     0x09
#define DATAGRAM_MOTIVE_SUBCOMMAND_MASK 0x0F
#define DATAGRAM_MOTIVE_SUBCOMMAND_FLAGS_MASK 0xF0

#define NUM_FUNCS 10

class Throttle: public OLCB_Datagram_Handler
{
 public:
  void init(void);
  void update(void);
  void datagramResult(bool accepted, uint16_t errorcode);
  bool processDatagram(void);

  void setSpeed(unsigned short speed, boolean foward);
  unsigned short getSpeed(void) {return _speed;}
  boolean getDirection(void) {return _direction;}
  void setFunction(byte funcID, boolean on);
  boolean getFunction(byte funcID) {return _functions[funcID];}
  void setAddress(unsigned int new_addy);
  unsigned int getAddress(void) {return _address;}
  boolean hasAddress(void) {if(_address) return true; else return false;}
  boolean isAttached(void) {return _attached;}

 private:
  unsigned short _speed;
  unsigned short _new_speed;
  
  boolean _direction;
  boolean _new_direction;
  
  boolean _functions[NUM_FUNCS];
  boolean _set_function;
  uint8_t _new_function_ID;
  boolean _new_function_val;

  unsigned int _address;
  unsigned int _new_address;
  
  boolean _attached;
  OLCB_Datagram _dg;
  
  byte _state;
};

#endif;
