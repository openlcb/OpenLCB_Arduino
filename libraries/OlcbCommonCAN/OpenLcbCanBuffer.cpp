#include <string.h>

// The following line is needed because the Arduino environment 
// won't search a library directory unless the library is included 
// from the top level file (this file)
#include "can.h"

#include "OpenLcbCan.h"
#include "OpenLcbCanInterface.h"
#include "OpenLcbCanBuffer.h"
#include "NodeID.h"
#include "EventID.h"

// for definiton, see
// http://openlcb.sf.net/trunk/documents/can/index.html
// 
// In the following masks, bit 0 of the frame is 0x10000000L
//

// bit 1
#define MASK_FRAME_TYPE 0x08000000L

// bit 17-28
#define MASK_SRC_ALIAS 0x00000FFFL

#define MASK_DST_ALIAS 0x000FFF000L

// bit 2-16
#define MASK_VARIABLE_FIELD 0x07FFF000L
#define SHIFT_VARIABLE_FIELD 12

// bit 2-4, at the top of the variable field
#define MASK_OPENLCB_FORMAT 0x07000L
#define SHIFT_OPENLCB_FORMAT 12

  void OpenLcbCanBuffer::init(uint16_t alias) {
    // set default header: extended frame w low priority
    flags.extended = 1;
    // all bits in header default to 1 except MASK_SRC_ALIAS
    id = 0x1FFFF000 | (alias & MASK_SRC_ALIAS);
  }

  // start of basic message structure

  void OpenLcbCanBuffer::setSourceAlias(uint16_t a) {
    id &= ~MASK_SRC_ALIAS;
    id = id | (a & MASK_SRC_ALIAS);
  }
  
  uint16_t OpenLcbCanBuffer::getSourceAlias() {
      return id & MASK_SRC_ALIAS;
  }
  uint16_t OpenLcbCanBuffer::getDestAlias() {
      return (id & MASK_DST_ALIAS ) >>12;
  }

  void OpenLcbCanBuffer::setFrameTypeCAN() {
    id &= ~MASK_FRAME_TYPE;     
  }
  
  bool OpenLcbCanBuffer::isFrameTypeCAN() {
    return (id & MASK_FRAME_TYPE) == 0x00000000L;
  }

  void OpenLcbCanBuffer::setFrameTypeOpenLcb() {
    id |= MASK_FRAME_TYPE;     
  }
  
  bool OpenLcbCanBuffer::isFrameTypeOpenLcb() {
    return (id & MASK_FRAME_TYPE) == MASK_FRAME_TYPE;
  }

  void OpenLcbCanBuffer::setVariableField(uint16_t f) {
    id &= ~MASK_VARIABLE_FIELD;
    uint32_t temp = f;  // ensure 32 bit arithmetic
    id |=  ((temp << SHIFT_VARIABLE_FIELD) & MASK_VARIABLE_FIELD);
  }

  uint16_t OpenLcbCanBuffer::getVariableField() {
    return (id & MASK_VARIABLE_FIELD) >> SHIFT_VARIABLE_FIELD;
  }
  
  // end of basic message structure
  
  // start of CAN-level messages
 
  void OpenLcbCanBuffer::setFrameTypeCAN(uint16_t alias, uint16_t var) {
    init(alias);
    setFrameTypeCAN();
    setVariableField(var);
    length=0;
  }

  void OpenLcbCanBuffer::setAMD(uint16_t alias,NodeID* nid) {
    setFrameTypeCAN(alias, AMD_VAR_FIELD);
    length=6;
    memcpy(data, nid->val, 6);
  }

  bool OpenLcbCanBuffer::isAMD(uint16_t alias) {
    return isFrameTypeCAN() && (getVariableField() == AMD_VAR_FIELD)
                && (alias == getSourceAlias());
  }

  void OpenLcbCanBuffer::setAMR(uint16_t alias,NodeID* nid) {
    setFrameTypeCAN(alias, AMR_VAR_FIELD);
    length=6;
    memcpy(data, nid->val, 6);
  }

  bool OpenLcbCanBuffer::isAMR(uint16_t alias) {
    return isFrameTypeCAN() && (getVariableField() == AMR_VAR_FIELD)
                && (alias == getSourceAlias());
  }

  void OpenLcbCanBuffer::setCIM(uint8_t i, uint16_t testval, uint16_t alias) {
    uint16_t var =  (( (0x7-i) & 7) << 12) | (testval & 0xFFF); 
    setFrameTypeCAN(alias, var);
  }

  bool OpenLcbCanBuffer::isCIM() {
    return isFrameTypeCAN() && (getVariableField()&0x7000) >= 0x4000;
  }

  void OpenLcbCanBuffer::setRIM(uint16_t alias) {
    setFrameTypeCAN(alias, RIM_VAR_FIELD);
  }

  bool OpenLcbCanBuffer::isRIM() {
      return isFrameTypeCAN() && getVariableField() == RIM_VAR_FIELD;
  }


  // end of CAN-level messages
  
  // start of OpenLCB format support
  
  uint16_t OpenLcbCanBuffer::getOpenLcbFormat() {
      return (getVariableField() & MASK_OPENLCB_FORMAT) >> SHIFT_OPENLCB_FORMAT;
  }

  void OpenLcbCanBuffer::setOpenLcbFormat(uint16_t i) {
      uint16_t now = getVariableField() & ~MASK_OPENLCB_FORMAT;
      setVariableField( ((i << SHIFT_OPENLCB_FORMAT) & MASK_OPENLCB_FORMAT) | now);
  }

  // is the variable field a destID?
  bool OpenLcbCanBuffer::isOpenLcDestIdFormat() {
      return ( getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_NON_DATAGRAM);
  }
  
  // is the variable field a stream ID?
  bool OpenLcbCanBuffer::isOpenLcbStreamIdFormat() {
      return ( getOpenLcbFormat() == MTI_FORMAT_STREAM_CODE);
  }
  
  bool OpenLcbCanBuffer::isMsgForHere(uint16_t alias) {
    return isForHere(alias) && isFrameTypeOpenLcb();
  }
  
  bool OpenLcbCanBuffer::isForHere(uint16_t alias) {  // includes frame level
    if (!isFrameTypeOpenLcb()) return true;
    uint16_t format = getOpenLcbFormat();
    if (format == MTI_FORMAT_UNADDRESSED_MTI) return true;
    else return alias == getDestAlias();
  }
  
  bool OpenLcbCanBuffer::isAddressedMessage() {
    uint16_t format = getOpenLcbFormat();
    if (format == MTI_FORMAT_UNADDRESSED_MTI) return false;
    else return true;
  }

  void OpenLcbCanBuffer::setOpenLcbMTI(uint16_t fmt, uint16_t mtiHeaderByte) {
        setFrameTypeOpenLcb();
        setVariableField(mtiHeaderByte);
        setOpenLcbFormat(fmt);  // order matters here
  }
  
  bool OpenLcbCanBuffer::isOpenLcbMTI(uint16_t fmt, uint16_t mtiHeaderByte) {
      return isFrameTypeOpenLcb() 
                && ( getOpenLcbFormat() == fmt )
                && ( (getVariableField()&~MASK_OPENLCB_FORMAT) == mtiHeaderByte );
  }
  
  // end of OpenLCB format and decode support
  
  // start of OpenLCB messages
  
  void OpenLcbCanBuffer::setPCEventReport(EventID* eid) {
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI,MTI_12_PC_EVENT_REPORT);
    length=8;
    loadFromEid(eid);
  }
  
  bool OpenLcbCanBuffer::isPCEventReport() {
      return isOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI, MTI_12_PC_EVENT_REPORT);
  }

  void OpenLcbCanBuffer::setLearnEvent(EventID* eid) {
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI,MTI_12_LEARN_EVENT);
    length=8;
    loadFromEid(eid);
  }

  bool OpenLcbCanBuffer::isLearnEvent() {
      return isOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI, MTI_12_LEARN_EVENT);
  }

  void OpenLcbCanBuffer::setInitializationComplete(uint16_t alias, NodeID* nid) {
    nodeAlias = alias;
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI,MTI_12_INITIALIZATION_COMPLETE);
    length=6;
    memcpy(data, nid->val, 6);
    //data[0] = nid->val[0];
    //data[1] = nid->val[1];
    //data[2] = nid->val[2];
    //data[3] = nid->val[3];
    //data[4] = nid->val[4];
    //data[5] = nid->val[5];
  }
  
  bool OpenLcbCanBuffer::isInitializationComplete() {
      return isOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI, MTI_12_INITIALIZATION_COMPLETE);
  }
  
  void OpenLcbCanBuffer::getEventID(EventID* evt) {
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
  
  void OpenLcbCanBuffer::getNodeID(NodeID* nid) {
    memcpy(nid->val, data, 6);
    //nid->val[0] = data[0];
    //nid->val[1] = data[1];
    //nid->val[2] = data[2];
    //nid->val[3] = data[3];
    //nid->val[4] = data[4];
    //nid->val[5] = data[5];
  }
  
  bool OpenLcbCanBuffer::matchesNid(NodeID* nid) {
        return 
            nid->val[0] == data[0] &&
            nid->val[1] == data[1] &&
            nid->val[2] == data[2] &&
            nid->val[3] == data[3] &&
            nid->val[4] == data[4] &&
            nid->val[5] == data[5];
  }

  bool OpenLcbCanBuffer::isVerifyNIDglobal() {
      return isOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI, MTI_12_VERIFY_NID_GLOBAL);
  }

  bool OpenLcbCanBuffer::isVerifyNID(uint16_t nida) {
      if (nida != (getVariableField()&0xFFF) ) return false;
      if (getOpenLcbFormat() != MTI_FORMAT_ADDRESSED_NON_DATAGRAM) return false;
      if (length == 0) return false;
      if (data[0] != MTI_8_VERIFY_NID) return false;
      return true;
  }

  void OpenLcbCanBuffer::setVerifiedNID(NodeID* nid) {
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI,MTI_12_VERIFIED_NID);
    length=6;
    memcpy(data, nid->val, 6);
    //data[0] = nid->val[0];
    //data[1] = nid->val[1];
    //data[2] = nid->val[2];
    //data[3] = nid->val[3];
    //data[4] = nid->val[4];
    //data[5] = nid->val[5];
  }

  bool OpenLcbCanBuffer::isVerifiedNID()
  {
    return isOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI, MTI_12_VERIFIED_NID);
  }

  void OpenLcbCanBuffer::setOptionalIntRejected(OpenLcbCanBuffer* rcv, uint16_t code) {
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM,rcv->getSourceAlias());
    length=5;
    data[0] = MTI_8_OPTION_INT_REJECTED;
    // make 1 byte MTI into 2 byte form
    uint16_t mti = (rcv->data[0]) << 4;
    data[1] = ((mti>>8)&0xFF) | 0x10;  // add addressed bit
    data[2] = mti&0xFF;

    data[3] = (code>>8)&0xFF;
    data[4] =  code    &0xFF;
  }

  bool OpenLcbCanBuffer::isIdentifyConsumers() {
      return isOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI, MTI_12_IDENTIFY_CONSUMERS);
  }

  void OpenLcbCanBuffer::setConsumerIdentified(EventID* eid) {
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI,MTI_12_CONSUMER_IDENTIFIED);
    length=8;
    loadFromEid(eid);
  }

  void OpenLcbCanBuffer::setConsumerIdentifyRange(EventID* eid, EventID* mask) {
    // does send a message, but not complete yet - RGJ 2009-06-14
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI,MTI_12_IDENTIFY_CONSUMERS_RANGE);
    length=8;
    loadFromEid(eid);
  }

  bool OpenLcbCanBuffer::isIdentifyProducers() {
      return isOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI, MTI_12_IDENTIFY_PRODUCERS);
  }

  void OpenLcbCanBuffer::setProducerIdentified(EventID* eid) {
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI,MTI_12_PRODUCER_IDENTIFIED);
    length=8;
    loadFromEid(eid);
  }

  void OpenLcbCanBuffer::setProducerIdentifyRange(EventID* eid, EventID* mask) {
    // does send a message, but not complete yet - RGJ 2009-06-14
    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI,MTI_12_IDENTIFY_PRODUCERS_RANGE);
    length=8;
    loadFromEid(eid);
  }

  bool OpenLcbCanBuffer::isIdentifyEventsGlobal() {
      return isOpenLcbMTI(MTI_FORMAT_UNADDRESSED_MTI, MTI_12_IDENTIFY_EVENTS_GLOBAL);
  }

  bool OpenLcbCanBuffer::isIdentifyEvents(uint16_t nida) {
      if (nida != (getVariableField()&0xFFF) ) return false;
      if (getOpenLcbFormat() != MTI_FORMAT_ADDRESSED_NON_DATAGRAM) return false;
      if (length == 0) return false;
      if (data[0] != MTI_8_IDENTIFY_EVENTS) return false;
      return true;
  }

  void OpenLcbCanBuffer::loadFromEid(EventID* eid) {
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
  bool OpenLcbCanBuffer::isDatagram() {
      return isFrameTypeOpenLcb() 
                && ( (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM_ALL)
                        || (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM_FIRST)
                        || (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM_MID)
                        || (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM_LAST))
                && (nodeAlias == (getVariableField()&0xFFF) );
  }
  // just checks 1st, assumes datagram already checked.
  bool OpenLcbCanBuffer::isLastDatagram() {
      return (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM_LAST)
            || (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM_ALL);
  }
