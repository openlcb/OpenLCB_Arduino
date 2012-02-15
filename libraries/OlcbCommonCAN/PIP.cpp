#include <string.h>

#include "PIP.h"

#include "LinkControl.h"
#include "OpenLcbCanBuffer.h"
#include "OpenLcbCan.h"

#include "logging.h"


PIP::PIP(uint8_t* by, OpenLcbCanBuffer* b, LinkControl* li) {
      bytes = by;
      buffer = b;
      link = li;
      queued = false;
  }
  
  void PIP::check() {
    if (queued) {
        if (OpenLcb_can_xmt_ready(buffer)) {
            buffer->setVariableField(dest);
            buffer->setFrameTypeOpenLcb();
            buffer->setOpenLcbFormat(MTI_FORMAT_ADDRESSED_NON_DATAGRAM);
            buffer->data[0] = 0x2F;
            buffer->data[1] = bytes[0];
            buffer->data[2] = bytes[1];
            buffer->data[3] = bytes[2];
            buffer->data[4] = bytes[3];
            buffer->data[5] = bytes[4];
            buffer->data[6] = bytes[5];
            buffer->length = 7;
            OpenLcb_can_queue_xmt_immediate(buffer);  // checked previously
            queued = false;
        }
    }
    return;
  }
    
  bool PIP::receivedFrame(OpenLcbCanBuffer* rcv) {
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

