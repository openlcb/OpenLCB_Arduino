#ifndef LinkControl_h
#define LinkControl_h

class OpenLcbCanBuffer;
class NodeID;

#include <stdint.h>

/**
 * Controls a OpenLcb S9.6 CAN link.
 * <p>
 * Once configured and started, this will handle all exchanges
 * on the CAN link until the link is operational.
 * It will then listen for errors, conflicts, etc, and handle
 * them as best it can.
 * <ul>
 * <li> Run the CIM/RIM algorithm to get an NID alias
 * <li> Send Initialization Complete
 * <li> Declare link up.
 * </ul>
 * <p>
 * Also processes received messages during normal operation to:
 * <ul>
 * <li>Handle problems with alias assignment
 * <li>Repond to Verify requests
 * </ul>
 * 
 * Does not process the Producer/Consumer messages, or any other protocol.
 * Those should be initialized _only_ after this object reports link up.
 * @author Bob Jacobsen   Copyright 2009
 * 
 */
class LinkControl {
    
 public:
 /**
  * Create the link controller, providing the transmit
  * buffer to be used to send frames, and the NodeID of this
  * node.
  */
  LinkControl(OpenLcbCanBuffer* b, NodeID* n);

  /**
   * Invoke once to configure an object of this type.
   * Separate from the ctor of the object, this 
   * should be invoked after the CAN hardware has been
   * initialized. Calling it again starts the initialization
   * again.
   */
  void reset();
  
  /**
   * Invoke repeatedly and often, this works with the
   * CAN interface to send needed startup messages.
   * Only needed when when linkInitialized() is false.
   */
  void check();

  /**
   * When a CAN frame is received _after_ initialization is 
   * complete (e.g. when linkInitialized() is true), it should
   * be transferred to the LinkControl object via this method
   * so that it can check for errors and handle Verify requests.
   */
  void receivedFrame(OpenLcbCanBuffer* rcv);

  /**
   * Check if the link startup procedure has completed OK.
   */
  bool linkInitialized();
  
  /**
   * Get the alias value
   */
  unsigned int getAlias();
  
  protected: 
  /**
   * Send the next CIM message.  Return true it you can/did,
   * false if you didn't.
   */
  bool sendCIM(int i);
  
  /**
   * Send an RIM message.  Return true it you can/did,
   * false if you didn't.
   */
  bool sendRIM();
  
  /**
   * Send the InitializationComplete when everything is OK.
   *  Return true it you can/did,
   * false if you didn't.
   */
  bool sendInitializationComplete();
  
  /**
   * Move to the next possible alias value during CIM/RIM resolution.
   */
  void nextAlias();
  
  OpenLcbCanBuffer* txBuffer;
  NodeID* nid;
  unsigned long timer; // used to wait for specific times
  uint32_t lfsr1, lfsr2;  // PRNG sequence value: lfsr1 is upper 24 bits, lfsr2 lower
  uint8_t state; // internal state counter, starts at zero
};

#endif
