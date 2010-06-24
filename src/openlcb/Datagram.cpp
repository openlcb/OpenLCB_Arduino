// makes this an Arduino file
#include "WConstants.h"

#include "OpenLcbCan.h"
#include "OpenLcbCanBuffer.h"
#include "LinkControl.h"
#include "Datagram.h"

#include "logging.h"

Datagram::Datagram(OpenLcbCanBuffer* b, unsigned int (*cb)(uint8_t tbuf[DATAGRAM_LENGTH], unsigned int length,  unsigned int from), LinkControl* ln) {
      buffer = b;
      link = ln;
      callback = cb; 
      reserved = false;
      sendcount = -1;
      rptr = rbuf;
}


uint8_t* Datagram::getTransmitBuffer() {
    if (reserved) return 0;
    reserved = true;
    return tbuf;
}

void Datagram::sendTransmitBuffer(int length, unsigned int destNIDa) {
    if (!reserved) {
        log("error: Datagram::sendTransmitBuffer when not reserved");
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
      buffer->setVariableField(dest);
      buffer->setFrameTypeOpenLcb();
      buffer->setOpenLcbFormat(MTI_FORMAT_ADDRESSED_DATAGRAM);
      buffer->length = min(sendcount,8);
      for (int i = 0; i<buffer->length; i++)
          buffer->data[i] = *(tptr++);
      sendcount = sendcount - buffer->length;
      // if hit zero this time, done
      if (sendcount == 0) {
          sendcount = -1;
          buffer->setOpenLcbFormat(MTI_FORMAT_ADDRESSED_DATAGRAM_LAST);
      }
      // and send it
      OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
      // and wait for reply
    }
  }
}

void Datagram::receivedFrame(OpenLcbCanBuffer* rcv) {
    // check for datagram reply, which can free buffer
    if ( (rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM, link->getAlias()) )
         && (dest == rcv->getSourceAlias()) 
         && (rcv->length == 1) ) {
        // for this node, check meaning
        if (rcv->data[0] == (MTI_DATAGRAM_RCV_OK>>4)&0xFF ) { // datagram ACK
            // this is to us, check data
            // release reserve
            reserved = false;
        } else if (rcv->data[0] == (MTI_DATAGRAM_REJECTED>>4)&0xFF ) { // datagram NAK
            // set up for resend
            sendcount = resendcount;
            tptr = tbuf;
            first = true;         
        }
    }
    // check for datagram fragment received
    // TODO: Check for correct source (prevent overlapping reception)
    if ( (true)   // saved for address check
          && ( (rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM, link->getAlias()) )
                || (rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_LAST, link->getAlias()) ) ) ) {
         // this is a datagram fragment, store into the buffer
         for (int i=0; i<rcv->length; i++) {
            *(rptr++) = rcv->data[i];
         }
         // is the end of the datagram?
         if (rcv->isOpenLcbMTI(MTI_FORMAT_ADDRESSED_DATAGRAM_LAST, link->getAlias()) ) {
            // 
            unsigned int length = rptr-rbuf;
            // callback
            int result = (*callback)(rbuf, length, rcv->getSourceAlias());
            rptr = rbuf;
            if (result == 0) {
                // send OK; done immediately with wait
                // TODO: Need a more robust method here
                // load buffer
                buffer->setVariableField(rcv->getSourceAlias());
                buffer->setFrameTypeOpenLcb();
                buffer->setOpenLcbFormat(MTI_FORMAT_ADDRESSED_NON_DATAGRAM);
                buffer->data[0] = MTI_DATAGRAM_RCV_OK>>4;
                buffer->length = 1;
                OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued _WITHOUT_ prior check
            } else {
                // not OK, send reject; done immediately with wait
                // TODO: Need a more robust method here
                buffer->setVariableField(rcv->getSourceAlias());
                buffer->setFrameTypeOpenLcb();
                buffer->setOpenLcbFormat(MTI_FORMAT_ADDRESSED_NON_DATAGRAM);
                buffer->data[0] = MTI_DATAGRAM_REJECTED>>4;
                buffer->data[1] = (result>>8)&0xFF;
                buffer->data[2] = result&0xFF;
                buffer->length = 3;
                OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued _WITHOUT_ prior check
            }
         }
    }
}
