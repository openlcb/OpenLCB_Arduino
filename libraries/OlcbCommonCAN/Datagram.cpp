#include <string.h>

#include "OpenLcbCan.h"
#include "OpenLcbCanBuffer.h"
#include "LinkControl.h"
#include "Datagram.h"

#include<stdio.h>

#include "logging.h"


Datagram::Datagram(OpenLcbCanBuffer* b, unsigned int (*cb)(uint8_t tbuf[DATAGRAM_LENGTH], unsigned int length,  unsigned int from), LinkControl* ln) {
      buffer = b;
      link = ln;
      callback = cb; 
      reservedXmt = false;
      sendcount = -1;
      rptr = rbuf;
      receiving = false;
}


uint8_t* Datagram::getTransmitBuffer() {
    if (reservedXmt) return 0;
    reservedXmt = true;
    return tbuf;
}

void Datagram::sendTransmitBuffer(int length, unsigned int destNIDa) {
    if (!reservedXmt) {
        //logstr("error: Datagram::sendTransmitBuffer when not reserved");
        return;
    }
    sendcount = length;
    resendcount = length;
    tptr = tbuf;
    first = true;
    dest = destNIDa;
}

void Datagram::check() {
  // see if any datagram segments are waiting to send
  if (sendcount >= 0) { // 0 is valid length
    // check if can send now
    if (OpenLcb_can_xmt_ready(buffer)) {
      // load buffer
      if (first)
        buffer->setOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_FIRST, dest);
      else
        buffer->setOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_MID, dest);
      buffer->length = sendcount<8 ? sendcount : 8;
      for (int i = 0; i<buffer->length; i++)
          buffer->data[i] = *(tptr++);
      sendcount = sendcount - buffer->length;
      // if hit zero this time, done
      if (sendcount == 0) {
          sendcount = -1;
          if (first)
              buffer->setOpenLcbFormat(MTI_FORMAT_ADDRESSED_DATAGRAM_ALL);
          else
              buffer->setOpenLcbFormat(MTI_FORMAT_ADDRESSED_DATAGRAM_LAST);
      }
      // and send it
      first = false;
      OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
      // and wait for reply
    }
  }
}

bool Datagram::receivedFrame(OpenLcbCanBuffer* rcv) {
    // conditionally check for link-level frames that stop reception
    if (receiving) {
        if (rcv->isAMR(fromAlias) || rcv->isAMD(fromAlias)) {
            // yes, abort reception
            rptr = rbuf;
            receiving = false;
            return false;
        }
    }
    // check for datagram reply, which can free buffer
    if ( (rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM, link->getAlias()) )  // addressed to here
         // TODO also need to check that this is _from_ the node that's sending the datagram
         // as a redundant check (shouldn't be anybody else sending a reply right now, though)
         ) {
        // for this node, check meaning
        if (rcv->data[0] == MTI_8_DATAGRAM_RCV_OK ) { // datagram ACK
            // release reserve
            reservedXmt = false;
            return true;
        } else if (rcv->data[0] == MTI_8_DATAGRAM_REJECTED ) { // datagram NAK
            // check permanent or temporary
            if (rcv->length < 2 || rcv->data[1] & (DATAGRAM_REJECTED_RESEND_MASK >> 8)) {
                // temporary, set up for resend
                sendcount = resendcount;
                tptr = tbuf;
                first = true;    
                return true;
            } else {
                // permanent, drop; nothing else to do?
                // TODO signal permanent error somehow     
                // release reserve
                reservedXmt = false;
                return true;
            }
        }
        return false;
    }
    // check for datagram fragment received
    if (       rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_ALL, link->getAlias()) 
            || rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_FIRST, link->getAlias())
            || rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_MID, link->getAlias())
            || rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_LAST, link->getAlias())
          ) {
         // can this start reception, e.g. we're not already receiving?
         if (
               rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_ALL, link->getAlias())
            || rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_FIRST, link->getAlias()) 
            ) {
            if (receiving) {
                // already receiving, this is an error, reply if last
                if (rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_ALL, link->getAlias()) ) {
                    // TODO: Need a more robust scheduling method here
                    buffer->setOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM,rcv->getSourceAlias());
                    buffer->data[0] = MTI_8_DATAGRAM_REJECTED;
                    buffer->data[1] = (DATAGRAM_REJECTED_BUFFER_FULL>>8)&0xFF;
                    buffer->data[2] = DATAGRAM_REJECTED_BUFFER_FULL&0xFF;
                    buffer->length = 3;
                    OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued _WITHOUT_ prior check
                }
                return true;
                // TODO send error response some how; will return false be enough?
            } else {
                // not receiving, start reception and continue processing
                fromAlias = rcv->getSourceAlias();
                receiving = true;
            }
         } else {
            // not a start segment, make sure we're receiving
            if (!receiving) {
                // missed the start frame? if last, tell
                if (rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_ALL, link->getAlias())
                    || rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_LAST, link->getAlias())
                    ) {
                    // TODO: Need a more robust scheduling method here
                    buffer->setOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM,rcv->getSourceAlias());
                    buffer->data[0] = MTI_8_DATAGRAM_REJECTED;
                    buffer->data[1] = (DATAGRAM_REJECTED_OUT_OF_ORDER>>8)&0xFF;
                    buffer->data[2] = DATAGRAM_REJECTED_OUT_OF_ORDER&0xFF;
                    buffer->length = 3;
                    OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued _WITHOUT_ prior check
                }
                return true;
                // TODO send error response some how; will return false be enough?
            }
         }        
         // this is for us, is it part of a currently-being received datagram?
         if (fromAlias != rcv->getSourceAlias()) {
            // no - if last, send reject temporarily; done immediately with wait
            if (rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_ALL, link->getAlias())
                || rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_LAST, link->getAlias())
                ) {
                // TODO: Need a more robust scheduling method here
                buffer->setOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM,rcv->getSourceAlias());
                buffer->data[0] = MTI_8_DATAGRAM_REJECTED;
                buffer->data[1] = (DATAGRAM_REJECTED_BUFFER_FULL>>8)&0xFF;
                buffer->data[2] = DATAGRAM_REJECTED_BUFFER_FULL&0xFF;
                buffer->length = 3;
                OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued _WITHOUT_ prior check
            }
            return true;
         }
         // this is a datagram fragment, store into the buffer
         for (int i=0; i<rcv->length; i++) {
            *(rptr++) = rcv->data[i];
         }
         // is the end of the datagram?
         if (rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_ALL, link->getAlias()) 
            || rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_LAST, link->getAlias()) 
            ) {
            // 
            unsigned int length = rptr-rbuf;
            // callback; result is error code or zero
            unsigned int result = (*callback)(rbuf, length, rcv->getSourceAlias());
            rptr = rbuf;
            receiving = false;
            buffer->setOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM,rcv->getSourceAlias());
            if (result == 0) {
                // send OK; done immediately with wait
                // TODO: Need a more robust method here
                // load buffer
                buffer->data[0] = MTI_8_DATAGRAM_RCV_OK;
                buffer->length = 1;
            } else {
                // not OK, send reject; done immediately with wait
                // TODO: Need a more robust method here
                buffer->data[0] = MTI_8_DATAGRAM_REJECTED;
                if (result > 0) result = DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;
                buffer->data[1] = (result>>8)&0xFF;
                buffer->data[2] = result&0xFF;
                buffer->length = 3;
            }
            OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued _WITHOUT_ prior check
         }
         return true;
    }
    return false;
}
