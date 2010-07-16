#ifndef OpenLcbCan_h
#define OpenLcbCan_h

/**
 * OpenLCB CAN MTI format bits
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
 * Basic header MTI definitions for OpenLCB on CAN.
 * Low bits 0x00F on all of these automatically
 */
 
#define MTI_INITIALIZATION_COMPLETE     0x08F

#define MTI_VERIFY_NID                  0x0AF
#define MTI_VERIFY_NID_GLOBAL           0x0A0
#define MTI_VERIFIED_NID                0x0BF

#define MTI_IDENTIFY_CONSUMERS          0x24F
#define MTI_IDENTIFY_CONSUMERS_RANGE    0x25F
#define MTI_CONSUMER_IDENTIFIED         0x26F

#define MTI_IDENTIFY_PRODUCERS          0x28F
#define MTI_IDENTIFY_PRODUCERS_RANGE    0x29F
#define MTI_PRODUCER_IDENTIFIED         0x2AF

#define MTI_IDENTIFY_EVENTS             0x2BF

#define MTI_LEARN_EVENT                 0x2CF
#define MTI_PC_EVENT_REPORT             0x2DF

#define MTI_DATAGRAM_RCV_OK             0x4CF
#define MTI_DATAGRAM_REJECTED           0x4DF


#endif
