#ifndef PIP_h
#define PIP_h

/**
 * Class for handling Protocol Identification Protocol
 *
 */

class OpenLcbCanBuffer;
class LinkControl;

#include <stdint.h>

class PIP {
  public:

  PIP(uint8_t* bytes, OpenLcbCanBuffer* b, LinkControl* link);
  
  /**
   * Handle any routine processing that needs to be done.
   * Go through this in loop() to e.g. send pending messages
   */
  void check();
  
  /**
   * When a CAN frame is received, it should
   * be transferred to the PIP object via this method
   * so that it can handle the output protocol
   */
  bool receivedFrame(OpenLcbCanBuffer* rcv);
    
  private:
  LinkControl* link;
  OpenLcbCanBuffer* buffer;
  uint8_t* bytes;
  uint16_t dest;
  bool queued;
  
};

#endif
