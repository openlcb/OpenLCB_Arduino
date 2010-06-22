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
  void init();
  
  // start of basic message structure

  void setFrameTypeCAN();
  bool isFrameTypeCAN();
  
  void setFrameTypeOpenLcb();
  bool isFrameTypeOpenLcb();
  
  void setVariableField(unsigned int f);
  unsigned int getVariableField();
  
  void setSourceAlias(unsigned int a);
  unsigned int getSourceAlias();
  
  // end of basic message structure
  
  // start of CAN-level messages
  
  void setCIM(int i, unsigned int testval, unsigned int alias);
  bool isCIM();
  
  void setRIM(unsigned int alias);
  bool isRIM();

  // end of CAN-level messages
  
  // start of OpenLCB format support

  int getOpenLcbFormat();
  void setOpenLcbFormat(int i);
  
  bool isOpenLcbMtiFormat();
  bool isOpenLcDestIdFormat();
  bool isOpenLcbStreamIdFormat();

  bool isOpenLcbMTI(unsigned int fmt, unsigned int mti);
  
  // end of OpenLCB format support
  
  // start of OpenLCB messages
  
  void setInitializationComplete(unsigned int alias, NodeID* nid);
  bool isInitializationComplete();

  void setPCEventReport(EventID* eid);
  bool isPCEventReport();
  
  void getEventID(EventID* evt);
  void getNodeID(NodeID* nid);
  
  bool isVerifyNID();
  void setVerifiedNID(NodeID* nid);

  bool isIdentifyConsumers();
  
  void setConsumerIdentified(EventID* eid);
  
  // Mask uses an EventID data structure; 1 bit means mask out when routing
  void setConsumerIdentifyRange(EventID* eid, EventID* mask);

  bool isIdentifyProducers();

  void setProducerIdentified(EventID* eid);

  // Mask uses an EventID data structure; 1 bit means mask out when routing
  void setProducerIdentifyRange(EventID* eid, EventID* mask);

  bool isIdentifyEvents();

  bool isDatagram();
  bool isLastDatagram();
  
  private: 
  unsigned int nodeAlias;   // Initialization complete sets, all later use

  // service routine to copy content (0-7) to a previously-allocated Eid
  void loadFromEid(EventID* eid);
};

#endif
