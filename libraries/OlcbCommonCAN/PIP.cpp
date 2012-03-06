#include <string.h>

#include "PIP.h"

#include "LinkControl.h"
#include "OpenLcbCanBuffer.h"
#include "OpenLcbCan.h"

static LinkControl* link;
static OpenLcbCanBuffer* buffer;
static uint16_t dest;
static bool queued;

extern "C" {
extern uint8_t protocolIdentValue[6];
}

void PIP_setup(OpenLcbCanBuffer* b, LinkControl* li) {
      buffer = b;
      link = li;
      queued = false;
  }
  
  void PIP_check() {
    if (queued) {
        if (OpenLcb_can_xmt_ready(buffer)) {
            buffer->setOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM, dest);
            buffer->data[0] = 0x2F;
            memcpy( protocolIdentValue, (buffer->data)+1, 6); 
            buffer->length = 7;
            OpenLcb_can_queue_xmt_immediate(buffer);  // checked previously
            queued = false;
        }
    }
    return;
  }
    
  bool PIP_receivedFrame(OpenLcbCanBuffer* rcv) {
    if ( rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM, link->getAlias()) )  { 
        // for this node, check meaning
         if (rcv->data[0] == 0x2E ) { // PIP request
            queued = true;
            dest = rcv->getSourceAlias();
            return true;
        }
    }
    return false;
  }

