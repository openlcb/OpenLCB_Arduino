#ifndef OpenLcbCanBuffer_h
#define OpenLcbCanBuffer_h

class NodeID;
class EventID;
#include "OpenLcbCanInterface.h"

/**
 * Class to handle transforming OpenLCB (S9.6) frames to/from std CAN frames.
 * <p>
 * We're trying to localize the formating of frames to/from the node here,
 * so that only this class needs to change when/if the wire protocol changes.
 */
 class OpenLcbCanBuffer : public CanInterfaceBuffer {
  public: 
  
  // Initialize a buffer for transmission
  void init(uint16_t a);
  
  // start of basic message structure

  void setFrameTypeCAN();
  bool isFrameTypeCAN();
  
  void setFrameTypeOpenLcb();
  bool isFrameTypeOpenLcb();
  
  void setVariableField(uint16_t f);
  uint16_t getVariableField();
  
  void setSourceAlias(uint16_t a);
  uint16_t getSourceAlias();
  uint16_t getDestAlias();
  
  // end of basic message structure
  
  // start of CAN-level messages
  
  void setCIM(int i, uint16_t testval, uint16_t alias);
  bool isCIM();
  
  void setRIM(uint16_t alias);
  bool isRIM();

  // end of CAN-level messages
  
  // start of OpenLCB format support

  uint16_t getOpenLcbFormat();
  void setOpenLcbFormat(uint16_t i);
  
  bool isOpenLcbMtiFormat();
  bool isOpenLcDestIdFormat();
  bool isOpenLcbStreamIdFormat();

  void setOpenLcbMTI(uint16_t fmt, uint16_t mti);
  bool isOpenLcbMTI(uint16_t fmt, uint16_t mti);
  
  bool isMsgForHere(uint16_t alias);
  bool isAddressedMessage();

  // end of OpenLCB format support
  
  // start of OpenLCB messages
  
  void setInitializationComplete(uint16_t alias, NodeID* nid);
  bool isInitializationComplete();

  void setPCEventReport(EventID* eid);
  bool isPCEventReport();
  
  void setLearnEvent(EventID* eid);
  bool isLearnEvent();
  
  void getEventID(EventID* evt);
  void getNodeID(NodeID* nid);
  
  bool isVerifyNID(int nida);
  bool isVerifyNIDglobal();
  void setVerifiedNID(NodeID* nid);
  bool isVerifiedNID();

  void setOptionalIntRejected(OpenLcbCanBuffer* rcv);
  
  bool isIdentifyConsumers();
  
  void setConsumerIdentified(EventID* eid);
  
  // Mask uses an EventID data structure; 1 bit means mask out when routing
  void setConsumerIdentifyRange(EventID* eid, EventID* mask);

  bool isIdentifyProducers();

  void setProducerIdentified(EventID* eid);

  // Mask uses an EventID data structure; 1 bit means mask out when routing
  void setProducerIdentifyRange(EventID* eid, EventID* mask);

  bool isIdentifyEvents(int nida);
  bool isIdentifyEventsGlobal();

  bool isDatagram();
  bool isLastDatagram();
  
  private: 
  unsigned int nodeAlias;   // Initialization complete sets, all later use

  // service routine to copy content (0-7) to a previously-allocated Eid
  void loadFromEid(EventID* eid);
};

#endif
