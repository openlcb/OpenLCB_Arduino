#include <string.h>

#include "OLCB_CAN_Buffer.h"


  void OLCB_Buffer::init(uint16_t alias) {
    // set default header: extended frame w low priority
    flags.extended = 1;
    // all bits in header default to 1 except MASK_SRC_ALIAS
    id = 0x1FFFF000 | (alias & MASK_SRC_ALIAS);
  }

  // start of basic message structure

  void OLCB_Buffer::setSourceAlias(uint16_t a) {
    id &= ~MASK_SRC_ALIAS;
    id = id | (a & MASK_SRC_ALIAS);
  }
  
  uint16_t OLCB_Buffer::getSourceAlias() {
      return id & MASK_SRC_ALIAS;
  }

  void OLCB_Buffer::setFrameTypeCAN() {
    id &= ~MASK_FRAME_TYPE;     
  }
  
  bool OLCB_Buffer::isFrameTypeCAN() {
    return (id & MASK_FRAME_TYPE) == 0x00000000L;
  }

  void OLCB_Buffer::setFrameTypeOpenLcb() {
    id |= MASK_FRAME_TYPE;     
  }
  
  bool OLCB_Buffer::isFrameTypeOpenLcb() {
    return (id & MASK_FRAME_TYPE) == MASK_FRAME_TYPE;
  }

  void OLCB_Buffer::setVariableField(uint16_t f) {
    id &= ~MASK_VARIABLE_FIELD;
    uint32_t temp = f;  // ensure 32 bit arithmetic
    id |=  ((temp << SHIFT_VARIABLE_FIELD) & MASK_VARIABLE_FIELD);
  }

  uint16_t OLCB_Buffer::getVariableField() {
    return (id & MASK_VARIABLE_FIELD) >> SHIFT_VARIABLE_FIELD;
  }
  
  // end of basic message structure
  
  // start of CAN-level messages
 
#define RIM_VAR_FIELD 0x0700

  void OLCB_Buffer::setCIM(int i, uint16_t testval, uint16_t alias) {
    init(alias);
    setFrameTypeCAN();
    uint16_t var =  (( (0x7-i) & 7) << 12) | (testval & 0xFFF); 
    setVariableField(var);
    length=0;
  }

  bool OLCB_Buffer::isCIM() {
    return isFrameTypeCAN() && (getVariableField()&0x7000) >= 0x4000;
  }

  void OLCB_Buffer::setRIM(uint16_t alias) {
    init(alias);
    setFrameTypeCAN();
    setVariableField(RIM_VAR_FIELD);
    length=0;
  }

  bool OLCB_Buffer::isRIM() {
      return isFrameTypeCAN() && getVariableField() == RIM_VAR_FIELD;
  }


  // end of CAN-level messages
  
  // start of OpenLCB format support
  
  uint16_t OLCB_Buffer::getOpenLcbFormat() {
      return (getVariableField() & MASK_OPENLCB_FORMAT) >> SHIFT_OPENLCB_FORMAT;
  }

  void OLCB_Buffer::setOpenLcbFormat(uint16_t i) {
      uint16_t now = getVariableField() & ~MASK_OPENLCB_FORMAT;
      setVariableField( ((i << SHIFT_OPENLCB_FORMAT) & MASK_OPENLCB_FORMAT) | now);
  }

  // is the variable field a destID?
  bool OLCB_Buffer::isOpenLcDestIdFormat() {
      return ( getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_NON_DATAGRAM);
  }
  
  // is the variable field a stream ID?
  bool OLCB_Buffer::isOpenLcbStreamIdFormat() {
      return ( getOpenLcbFormat() == MTI_FORMAT_STREAM_CODE);
  }
  
  void OLCB_Buffer::setOpenLcbMTI(uint16_t fmt, uint16_t mtiHeaderByte) {
        setFrameTypeOpenLcb();
        setVariableField(mtiHeaderByte);
        setOpenLcbFormat(fmt);  // order matters here
  }
  
  bool OLCB_Buffer::isOpenLcbMTI(uint16_t fmt, uint16_t mtiHeaderByte) {
      return isFrameTypeOpenLcb() 
                && ( getOpenLcbFormat() == fmt )
                && ( (getVariableField()&~MASK_OPENLCB_FORMAT) == mtiHeaderByte );
  }

  // end of OpenLCB format and decode support
  
  // start of OpenLCB messages
  
  void OLCB_Buffer::setPCEventReport(OLCB_EventID* eid) {
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI,MTI_PC_EVENT_REPORT);
    length=8;
    loadFromEid(eid);
  }
  
  bool OLCB_Buffer::isPCEventReport() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_PC_EVENT_REPORT);
  }

  void OLCB_Buffer::setLearnEvent(OLCB_EventID* eid) {
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI,MTI_LEARN_EVENT);
    length=8;
    loadFromEid(eid);
  }

  bool OLCB_Buffer::isLearnEvent() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_LEARN_EVENT);
  }

  void OLCB_Buffer::setInitializationComplete(uint16_t alias, OLCB_NodeID* nid) {
    nodeAlias = alias;
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI,MTI_INITIALIZATION_COMPLETE);
    length=6;
    memcpy(data, nid->val, 6);
    //data[0] = nid->val[0];
    //data[1] = nid->val[1];
    //data[2] = nid->val[2];
    //data[3] = nid->val[3];
    //data[4] = nid->val[4];
    //data[5] = nid->val[5];
  }
  
  bool OLCB_Buffer::isInitializationComplete() {
      return isOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI, MTI_INITIALIZATION_COMPLETE);
  }
  
  void OLCB_Buffer::getEventID(OLCB_EventID* evt) {
    memcpy(evt->val, data, 8);
    //evt->val[0] = data[0];
    //evt->val[1] = data[1];
    //evt->val[2] = data[2];
    //evt->val[3] = data[3];
    //evt->val[4] = data[4];
    //evt->val[5] = data[5];
    //evt->val[6] = data[6];
    //evt->val[7] = data[7];
  }
  
  void OLCB_Buffer::getNodeID(OLCB_NodeID* nid) {
    memcpy(nid->val, data, 6);
    //nid->val[0] = data[0];
    //nid->val[1] = data[1];
    //nid->val[2] = data[2];
    //nid->val[3] = data[3];
    //nid->val[4] = data[4];
    //nid->val[5] = data[5];
    nid->alias = getSourceAlias();
  }
  
  bool OLCB_Buffer::isVerifyNID() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_VERIFY_NID);
  }
  
  void OLCB_Buffer::setVerifyNID(OLCB_NodeID* nid)
  {
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_VERIFY_NID);
    length = 6;
    memcpy(data, nid->val, 6);
  }

  bool OLCB_Buffer::isVerifyNIDglobal() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_VERIFY_NID_GLOBAL);
  }
  
  bool OLCB_Buffer::isVerifiedNID()
  {
    return isOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI, MTI_VERIFIED_NID);
  }

  void OLCB_Buffer::setVerifiedNID(OLCB_NodeID* nid) {
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI,MTI_VERIFIED_NID);
    length=6;
    memcpy(data, nid->val, 6);
    //data[0] = nid->val[0];
    //data[1] = nid->val[1];
    //data[2] = nid->val[2];
    //data[3] = nid->val[3];
    //data[4] = nid->val[4];
    //data[5] = nid->val[5];
  }

  bool OLCB_Buffer::isIdentifyConsumers() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_IDENTIFY_CONSUMERS);
  }

  void OLCB_Buffer::setConsumerIdentified(OLCB_EventID* eid) {
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI,MTI_CONSUMER_IDENTIFIED);
    length=8;
    loadFromEid(eid);
  }

  void OLCB_Buffer::setConsumerIdentifyRange(OLCB_EventID* eid, OLCB_EventID* mask) {
    // does send a message, but not complete yet - RGJ 2009-06-14
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI,MTI_IDENTIFY_CONSUMERS_RANGE);
    length=8;
    loadFromEid(eid);
  }

  bool OLCB_Buffer::isIdentifyProducers() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_IDENTIFY_PRODUCERS);
  }

  void OLCB_Buffer::setProducerIdentified(OLCB_EventID* eid) {
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI,MTI_PRODUCER_IDENTIFIED);
    length=8;
    loadFromEid(eid);
  }

  void OLCB_Buffer::setProducerIdentifyRange(OLCB_EventID* eid, OLCB_EventID* mask) {
    // does send a message, but not complete yet - RGJ 2009-06-14
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI,MTI_IDENTIFY_PRODUCERS_RANGE);
    length=8;
    loadFromEid(eid);
  }

  bool OLCB_Buffer::isIdentifyEvents() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_IDENTIFY_EVENTS);
  }

  void OLCB_Buffer::loadFromEid(OLCB_EventID* eid) {
    memcpy(data, eid->val, 8);
    //data[0] = eid->val[0];
    //data[1] = eid->val[1];
    //data[2] = eid->val[2];
    //data[3] = eid->val[3];
    //data[4] = eid->val[4];
    //data[5] = eid->val[5];
    //data[6] = eid->val[6];
    //data[7] = eid->val[7];
  }
  
  // general, but not efficient
  bool OLCB_Buffer::isDatagram() {
      return isFrameTypeOpenLcb() 
                && ( (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM)
                        || (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM_LAST))
                && (nodeAlias == getVariableField() );
  }
  // just checks 1st, assumes datagram already checked.
  bool OLCB_Buffer::isLastDatagram() {
      return (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM_LAST);
  }
