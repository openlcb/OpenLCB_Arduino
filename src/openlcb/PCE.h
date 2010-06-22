#ifndef PCE_h
#define PCE_h

/**
 * Class for handling P/C Events.
 *
 * This combines Producer and Consumer for now;
 * perhaps they need to be refactored separately later.
 *<p>
 * Basic state machine handles e.g. Request Producer/Consumer/event
 * messages for you, including at initialization.
 */

#include "EventID.h"
#include "Event.h"

class NodeID;
class OpenLcbCanBuffer;

class PCE {
  public:

  PCE(Event* c, int nC, Event* p, int nP, OpenLcbCanBuffer* b, NodeID* nid, void (*callback)(int i));
  
  /**
   * Produce the ith event
   * 
   * return true if done; return false to require retry later
   */
  void produce(int i);
  
  /**
   * Handle any routine processing that needs to be done.
   * Go through this in loop() to e.g. send pending messages
   */
  void check();
  
  /**
   * When a CAN frame is received, it should
   * be transferred to the PCER object via this method
   * so that it can handle the verification protocol.
   */
  void receivedFrame(OpenLcbCanBuffer* rcv);
  
  /**
   * A new event has been defined, and we should
   * do the necessary notifications, etc on the OpenLCB link
   *
   * index is the 0-based index of the newly defined
   * event in the array provided to the ctor earlier.
   */
  void newConsumedEvent(int index);
  
  /**
   * A new event has been defined, and we should
   * do the necessary notifications, etc on the OpenLCB link
   *
   * index is the 0-based index of the newly defined
   * event in the array provided to the ctor earlier.
   */
  void newProducedEvent(int index);
  
  private:
  Event* consumed;  // array
  Event* produced;  // array
  int nConsumed;
  int nProduced;
  OpenLcbCanBuffer* buffer;
  NodeID* nid;
  void (*callback)(int i);   // void callback(int index) pointer

  int sendProducer; // index of next producer identified event to send, or -1
  int sendConsumer; // index of next consumer identified event to send, or -1
};

#endif
