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
  
  //*****
  // The "getRead", "getWrite" and "reset" methods
  // are a very simple interface to the actual
  // device.  Redo them for the specific memory
  // map being used.
  // (We're trying for now to avoid virtual methods)
  // ****
  
  Configuration(Datagram* datagramHandler, Stream *streamHandler,
                        const uint8_t (*getRead)(int address, int space),
                        void (*getWrite)(int address, int space, uint8_t val),
                        void (*restart)()
            );
            
  void check(); 
  int receivedDatagram(uint8_t* data, int length, unsigned int from);
  
  private:
  void processRead(uint8_t* data, int length);
  void processWrite(uint8_t* data, int length);
  void processCmd(uint8_t* data, int length);
  
  Datagram* dg;
  Stream* str;
  uint8_t buffer[DATAGRAM_LENGTH];
  int length;
  unsigned int from;
  bool request;
  
  uint32_t getAddress(uint8_t* data);
  int decodeLen(uint8_t* data);
  int decodeSpace(uint8_t* data);
  
  const uint8_t (*getRead)(int address, int space);
  void (*getWrite)(int address, int space, uint8_t val);
  void (*restart)();
 
};

#endif
