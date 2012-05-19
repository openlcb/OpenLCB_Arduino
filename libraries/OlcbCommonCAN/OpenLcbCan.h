#ifndef OpenLcbCan_h
#define OpenLcbCan_h

/**
 * OpenLCB CAN Frame Transport
 */

#define RIM_VAR_FIELD 0x0700
#define AMD_VAR_FIELD 0x0701
#define AME_VAR_FIELD 0x0702
#define AMR_VAR_FIELD 0x0703

/**
 * OpenLCB CAN MTI format (3 bits)
 */
#define MTI_FORMAT_UNADDRESSED_MTI         0    
#define MTI_FORMAT_RESERVED                1
//
//
#define MTI_FORMAT_ADDRESSED_DATAGRAM_ALL   2    
#define MTI_FORMAT_ADDRESSED_DATAGRAM_FIRST 3    
#define MTI_FORMAT_ADDRESSED_DATAGRAM_MID   4    
#define MTI_FORMAT_ADDRESSED_DATAGRAM_LAST  5    
#define MTI_FORMAT_ADDRESSED_NON_DATAGRAM   6    
#define MTI_FORMAT_STREAM_CODE              7    


/**
 * Basic 12-bit header MTI definitions for OpenLCB on CAN.
 * See the MtiAllocations.ods document for allocations.
 *
 * Note: This is just the low 12 bits, and does not include
 * 0-7 format MTI format field just above this.
 */
 
#define MTI_12_INITIALIZATION_COMPLETE     0x087

#define MTI_12_VERIFY_NID_GLOBAL           0x8A7
#define MTI_12_VERIFIED_NID                0x8B7

#define MTI_12_IDENTIFY_CONSUMERS          0xA4F
#define MTI_12_IDENTIFY_CONSUMERS_RANGE    0x25F
#define MTI_12_CONSUMER_IDENTIFIED         0x26B

#define MTI_12_IDENTIFY_PRODUCERS          0xA8F
#define MTI_12_IDENTIFY_PRODUCERS_RANGE    0x29F
#define MTI_12_PRODUCER_IDENTIFIED         0x2AB

#define MTI_12_IDENTIFY_EVENTS_GLOBAL      0xAB7

#define MTI_12_LEARN_EVENT                 0xACF
#define MTI_12_PC_EVENT_REPORT             0xADF

/**
 * basic 8-bit Message Type byte values (from data[0])
 * for MTI_FORMAT_ADDRESSED_NON_DATAGRAM addressed messages.
 */

#define MTI_8_VERIFY_NID                  0x0A

#define MTI_8_IDENTIFY_EVENTS             0x2B

#define MTI_8_OPTION_INT_REJECTED         0x0C

#define MTI_8_DATAGRAM_RCV_OK             0x4C
#define MTI_8_DATAGRAM_REJECTED           0x4D

#endif
