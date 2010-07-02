// makes this an Arduino file
#include "WConstants.h"
#include <string.h>

#include "PCE.h"

#include "EventID.h"
#include "NodeID.h"
#include "OpenLcbCanBuffer.h"

#include "logging.h"

// Mark as waiting to have Identify sent
#define IDENT_FLAG 0x01
// Mark produced event for send
#define PRODUCE_FLAG 0x02
// Mark entry as really empty, ignore
#define EMPTY_FLAG 0x04
// Mark entry to written from next learn message
#define LEARN_FLAG 0x08
// Mark entry to send a learn message
#define TEACH_FLAG 0x10

PCE::PCE(Event* c, int nC, Event* p, int nP, OpenLcbCanBuffer* b, NodeID* n, void (*cb)(int i)) {
      consumed = c;
      nConsumed = nC;
      produced = p;
      nProduced = nP;
      buffer = b;
      nid = n;
      callback = cb;
      
      // mark as needing transmit of IDs, otherwise not interesting
        for (int i = 0; i < nProduced; i++) {
          produced[i].flags = IDENT_FLAG;
        }
        sendProducer = 0;
        for (int i = 0; i < nConsumed; i++) {
          consumed[i].flags = IDENT_FLAG;
        }
        sendConsumer = 0;        
      
  }
  
  void PCE::produce(int i) {
    // mark for production
    produced[i].flags |= PRODUCE_FLAG;
    sendProducer = min(sendProducer, i);
  }
  
  void PCE::check() {
     // see in any replies are waiting to send
     while (sendProducer < nProduced) {
         // OK to send, see if marked for some cause
         if (produced[sendProducer].flags & IDENT_FLAG) {
           produced[sendProducer].flags &= ~IDENT_FLAG;    // reset flag
           buffer->setProducerIdentified(&produced[sendProducer]);
           OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           break; // only send one from this loop
         } else if (produced[sendProducer].flags & PRODUCE_FLAG) {
           produced[sendProducer].flags &= ~PRODUCE_FLAG;    // reset flag
           buffer->setPCEventReport(&produced[sendProducer]);
           OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           break; // only send one from this loop
         } else if (produced[sendProducer].flags & TEACH_FLAG) {
           produced[sendProducer].flags &= ~TEACH_FLAG;    // reset flag
           buffer->setLearnEvent(&produced[sendProducer]);
           OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           break; // only send one from this loop
         } else {
           // just skip
           sendProducer++;
         }
     }
     
     while (sendConsumer < nConsumed) {
       if (OpenLcb_can_xmt_ready(buffer)) {
         // OK to send, see if marked for some cause
         if (consumed[sendConsumer].flags & IDENT_FLAG) {
           consumed[sendConsumer].flags &= ~IDENT_FLAG;    // reset flag
           buffer->setConsumerIdentified(&consumed[sendConsumer++]);
           OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           break; // only send one from this loop
         } else if (consumed[sendConsumer].flags & TEACH_FLAG) {
           consumed[sendConsumer].flags &= ~TEACH_FLAG;    // reset flag
           buffer->setLearnEvent(&consumed[sendConsumer++]);
           OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           break; // only send one from this loop
         } else {
           // just skip
           sendConsumer++;
         }
       }
     }
  }
  
  void PCE::newProducedEvent(int index) {
    produced[index].flags |= IDENT_FLAG;
  }

  void PCE::newConsumedEvent(int index) {
    consumed[index].flags |= IDENT_FLAG;
  }
  
  void PCE::markToLearnP(int index, bool mark) {
    if (mark)
        produced[index].flags |= LEARN_FLAG;
    else
        produced[index].flags &= ~LEARN_FLAG;
  }
  
  void PCE::sendTeachP(int index) {
    produced[index].flags |= TEACH_FLAG;
    sendProducer = min(sendProducer, index);
  }
  
  void PCE::markToLearnC(int index, bool mark) {
    if (mark)
        consumed[index].flags |= LEARN_FLAG;
    else
        consumed[index].flags &= ~LEARN_FLAG;
  }
  
  void PCE::sendTeachC(int index) {
    consumed[index].flags |= TEACH_FLAG;
    sendConsumer = min(sendConsumer, index);
  }
  
  void PCE::receivedFrame(OpenLcbCanBuffer* rcv) {
    if (rcv->isIdentifyConsumers()) {
        // see if we consume the listed event
        Event event;
        rcv->getEventID(&event);
        int index = event.findIndexInArray(consumed, nConsumed);
        if ( -1 != index) {
           // yes, we have to reply with ConsumerIdentified
           consumed[index].flags |= IDENT_FLAG;
           sendConsumer = min(sendConsumer, index);
        }
    } else if (rcv->isIdentifyProducers()) {
        // see if we produce the listed event
        Event event;
        rcv->getEventID(&event);
        int index = event.findIndexInArray(produced, nProduced);
        if ( -1 != index) {
           // yes, we have to reply with ProducerIdentified
           produced[index].flags |= IDENT_FLAG;
           sendProducer = min(sendProducer, index);
        }
    } else if (rcv->isIdentifyEvents()) {
        // See if addressed to us
       NodeID n;
       rcv->getNodeID(&n);
       if (n.equals(nid)) {
          // if so, send _all_ ProducerIndentfied, ConsumerIdentified
          // via the "check" periodic call
          for (int i = 0; i < nProduced; i++) {
            produced[i].flags |= IDENT_FLAG;
          }
          sendProducer = 0;
          for (int i = 0; i < nConsumed; i++) {
            consumed[i].flags |= IDENT_FLAG;
          }
          sendConsumer = 0;  
       }     
    } else if (rcv->isPCEventReport()) {
        // found a PC Event Report, see if we consume it
        Event event;
        rcv->getEventID(&event);
        int index;
        index = event.findIndexInArray(consumed, nConsumed);
        if ( -1 != index ) {
           // yes, notify our own code
           (*callback)(index);
        }        
    } else if (rcv->isLearnEvent()) {
        // found a teaching frame, apply to selected
        for (int i=0; i<nConsumed; i++) {
            if ( (consumed[i].flags & LEARN_FLAG ) != 0 ) {
                rcv->getEventID(consumed+i);
                consumed[i].flags |= IDENT_FLAG; // notify new eventID
                sendConsumer = min(sendConsumer, i);
            }
        }
        for (int i=0; i<nProduced; i++) {
            if ( (produced[i].flags & LEARN_FLAG ) != 0 ) {
                rcv->getEventID(produced+i);
                produced[i].flags |= IDENT_FLAG; // notify new eventID
                sendProducer = min(sendProducer, i);
            }
        }
    }
  }
  
