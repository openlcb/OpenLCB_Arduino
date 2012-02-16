#include <string.h>

#include "SNII.h"

#include "LinkControl.h"
#include "OpenLcbCanBuffer.h"
#include "OpenLcbCan.h"

#include "logging.h"

static OpenLcbCanBuffer* buffer;
static uint8_t count;
static LinkControl* link;
static uint16_t dest;
static uint8_t tptr;

const uint8_t getRead(uint32_t address, int space);

void SNII_setup(uint8_t c, OpenLcbCanBuffer* b, LinkControl* li) {
      count = c;
      buffer = b;
      link = li;
      tptr = count;
  }
  
void SNII_check() {
    if ( tptr < count ) {
        if (OpenLcb_can_xmt_ready(buffer)) {
            buffer->setOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM, dest);
            buffer->data[0] = 0x53;
            uint8_t i;
            for (i = 1; i<8; i++ ) {
                if (tptr >= count) break;
                buffer->data[i] = getRead(tptr++, 0xFC);
            }
            buffer->length = i;
            OpenLcb_can_queue_xmt_immediate(buffer);  // checked previously
        }
    }
    return;
}
    
bool SNII_receivedFrame(OpenLcbCanBuffer* rcv) {
    if ( rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM, link->getAlias()) )  { 
        // for this node, check meaning
        if (rcv->data[0] == 0x52 ) { // SCIP request
            tptr = 0;
            dest = rcv->getSourceAlias();
        }
        return true;
    }
    return false;
}

