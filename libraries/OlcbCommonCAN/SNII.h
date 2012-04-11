#ifndef SNII_h
#define SNII_h

/**
 * Handle Simple Node Identification Information protocol
 *
 */

class OpenLcbCanBuffer;
class LinkControl;

#include <stdint.h>

/**
* Call once to record constants and do needed setup
*/
void SNII_setup(uint8_t count, uint8_t var_comment_offset, OpenLcbCanBuffer* b, LinkControl* link);
  
/**
* Handle any routine processing that needs to be done.
* Go through this in loop() to e.g. send pending messages
*/
void SNII_check();
  
/**
* When a CAN frame is received, it should
* be transferred to the PIP object via this method
* so that it can handle the output protocol
*/
bool SNII_receivedFrame(OpenLcbCanBuffer* rcv);
      
#endif
