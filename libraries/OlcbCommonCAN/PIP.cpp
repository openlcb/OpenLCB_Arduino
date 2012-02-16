#include <string.h>

#include "PIP.h"

#include "LinkControl.h"
#include "OpenLcbCanBuffer.h"
#include "OpenLcbCan.h"

static LinkControl* link;
static OpenLcbCanBuffer* buffer;
static uint8_t* bytes;
static uint16_t dest;
static bool queued;


void PIP_setup(uint8_t* by, OpenLcbCanBuffer* b, LinkControl* li) {
      bytes = by;
      buffer = b;
      link = li;
      queued = false;
  }
  
  void PIP_check() {
    if (queued) {
        if (OpenLcb_can_xmt_ready(buffer)) {
            buffer->setOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM, dest);
            buffer->data[0] = 0x2F;
            memcpy( bytes, (buffer->data)+1, 6); 
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
         if (rcv->data[0] == (0x2E & 0xFF) ) { // PIP request
            queued = true;
            dest = rcv->getSourceAlias();
        }
        return true;
    }
    return false;
  }

