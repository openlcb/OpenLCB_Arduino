#ifndef Configuration_h
#define Configuration_h

/**
 * Class for handling Configuration protocol
 *
 * 
 */
#include <stdint.h>

#include "Datagram.h"
class Stream;

class Configuration {
  public:
  
  Configuration(Datagram* dg, Stream *str);
  
  void check(); 
  int receivedDatagram(uint8_t* data, int length);
  
  private:
  void processRead(uint8_t* data, int length);
  void processWrite(uint8_t* data, int length);
  void processCmd(uint8_t* data, int length);
  
  Datagram* dg;
  Stream* str;
  uint8_t buffer[DATAGRAM_LENGTH];
  int length;
  bool request;
  
  uint32_t getAddress(uint8_t* data);
  int decodeLen(uint8_t* data);
  int decodeSpace(uint8_t* data);
};

#endif
