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
#define MTI_FORMAT_SIMPLE_MTI               0    
#define MTI_FORMAT_COMPLEX_MTI              1
//
//
#define MTI_FORMAT_ADDRESSED_DATAGRAM       4    
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
 
#define MTI_INITIALIZATION_COMPLETE     0x087

#define MTI_VERIFY_NID_GLOBAL           0x0A7
#define MTI_VERIFIED_NID                0x0B7

#define MTI_IDENTIFY_CONSUMERS          0x24F
#define MTI_IDENTIFY_CONSUMERS_RANGE    0x25F
#define MTI_CONSUMER_IDENTIFIED         0x26B

#define MTI_IDENTIFY_PRODUCERS          0x28F
#define MTI_IDENTIFY_PRODUCERS_RANGE    0x29F
#define MTI_PRODUCER_IDENTIFIED         0x2AB

#define MTI_IDENTIFY_EVENTS_GLOBAL      0x2B7

#define MTI_LEARN_EVENT                 0x2CF
#define MTI_PC_EVENT_REPORT             0x2DF

/**
 * basic 8-bit Message Type byte values (from data[0])
 * for addressed messages.
 */

#define MTI_VERIFY_NID                  0x0A

#define MTI_IDENTIFY_EVENTS             0x2B

#define MTI_OPTION_INT_REJECTED         0x0C

#define MTI_DATAGRAM_RCV_OK             0x4C
#define MTI_DATAGRAM_REJECTED           0x4D

#endif
