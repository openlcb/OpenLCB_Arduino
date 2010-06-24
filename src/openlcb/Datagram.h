#ifndef Datagram_h
#define Datagram_h

/**
 * Class for handling Datagrams
 *
 * This combines Receiver and Transmitter for now;
 * perhaps they need to be refactored separately later.
 *<p>
 * Basic state machine handles transmission and reception.
 */

#define DATAGRAM_LENGTH 70
class OpenLcbCanBuffer;
class LinkControl;

class Datagram {
  public:

  /**
   * Ask for the transmit buffer. If
   * it's in use, indicated by returning a 0 pointer,
   * you must repeat the request later
   */
  uint8_t* getTransmitBuffer();
  
  /**
   * Send the filled transmit buffer.
   */
  void sendTransmitBuffer(int length, unsigned int destID);
  
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
  
  Datagram(OpenLcbCanBuffer* b, unsigned int (*callback)(uint8_t tbuf[DATAGRAM_LENGTH], unsigned int length, unsigned int from), LinkControl* link);
  
  private:
  OpenLcbCanBuffer* buffer;
  LinkControl* link;
  
  uint8_t tbuf[DATAGRAM_LENGTH];
  int sendcount;
  int resendcount;
  unsigned int dest;
  uint8_t* tptr;
  bool first;
  bool reserved;

  uint8_t rbuf[DATAGRAM_LENGTH];
  uint8_t* rptr;
  unsigned int (*callback)(uint8_t tbuf[DATAGRAM_LENGTH], unsigned int length, unsigned int from);   // void callback(int index) pointer
};

#endif
