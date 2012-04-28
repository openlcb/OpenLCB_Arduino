#ifndef __MYCONFIGHANDLER_H__
#define __MYCONFIGHANDLER_H__

#include "MyEventHandler.h"
#include "OLCB_Virtual_Node.h"
#include "OLCB_Datagram_Handler.h"

//XML CDI data, taken from Io_8P_8C_default.xml
static PROGMEM char cdixml[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<cdi>\n<identification>\n<manufacturer>Railstars Limited</manufacturer>\n<model>Io</model>\n<hardwareVersion>1.0</hardwareVersion>\n<softwareVersion>1.0</softwareVersion>\n</identification>\n<segment origin=\"0\" space=\"0\">\n<group offset=\"0\" replication=\"8\">\n<name>Produced Events</name>\n<description>The EventIDs for the outputs</description>\n<eventid/>\n<eventid/>\n</group>\n<group replication=\"8\">\n<name>Consumed Events</name>\n<description>The EventIDs for the inputs</description>\n<eventid/>\n<eventid/>\n</group>\n</segment>\n</cdi>\n";

/*{

  0x2e,  0x01,  0x92,  0xe2,  0xc1,  0xcf,  0x83,  0x2d,  0x1b,  0x7c,  0xa0,  0xf0,  0x30,  0x66,  0xc2,  0x32, 
  0x14,  0x40,  0xe2,  0x61,  0xc9,  0x84,  0x29,  0x3b,  0x2e,  0x2c,  0x99,  0xb1,  0x64,  0xcc,  0x30,  0x14, 
  0xd8,  0xb3,  0x4b,  0xc4,  0x24,  0x60,  0xcb,  0x90,  0x1d,  0x57,  0x66,  0x0c,  0x19,  0x73,  0xe1,  0xca, 
  0x89,  0x29,  0x27,  0x7c,  0x94,  0x18,  0xb2,  0x64,  0xc3,  0x99,  0x0b,  0x43,  0x4e,  0x9c,  0x11,  0x90, 
  0x61,  0xc9,  0x96,  0x25,  0x17,  0xa6,  0x4c,  0xf0,  0xa0,  0xcf,  0x6f,  0x50,  0x20,  0x1b,  0x90,  0xfd, 
  0x08,  0x2c,  0x01,  0x49,  0xf6,  0x59,  0x18,  0xc9,  0xc9,  0xc4,  0xa2,  0x09,  0x98,  0x70,  0x0f,  0x06, 
  0xa6,  0xd4,  0x86,  0x81,  0x33,  0xcb,  0x0d,  0xc4,  0x88,  0x0e,  0x43,  0x1e,  0xa4,  0x5c,  0x05,  0x71, 
  0x66,  0xcf,  0x8c,  0x4b,  0xc5,  0xa7,  0x5c,  0x05,  0xf4,  0x00,  0xcb,  0x13,  0x60,  0xc7,  0x85,  0x25, 
  0x33,  0x96,  0x8c,  0x19,  0x86,  0xc2,  0x02,  0x26,  0x21,  0x67,  0xa6,  0xcc,  0xd9,  0x32,  0x4d,  0x06, 
  0x04,  0xec,  0x39,  0xb1,  0x64,  0xce,  0x92,  0x1d,  0x5e,  0x44,  0x18,  0x10,  0x21,  0xe0,  0xcc,  0x81, 
  0x21,  0x63,  0xa6,  0x91,  0x84,  0x6f,  0x9b,  0x98,  0x73,  0x62,  0xcf,  0x95,  0x43,  0x1e,  0x30,  0x63, 
  0x56,  0x09,  0x5c,  0xf2,  0x98,  0x13,  0x53,  0x0e,  0x9c,  0x23,  0x00,  0x07,  0xa2,  0x5c,  0x02,  0x89, 
  0x87,  0x1d,  0xc3,  0x3e,  0xc0,  0x47,  0xa1,  0x18,  0x98,  0x70,  0x6d,  0x06,  0x26,  0x08,  0x88,  0x72, 
  0x9b,  0x46,  0xce,  0xe7,  0x90,  0xca,  0xbd,  0x98,  0x0f,  0x0f,  0xc0,  0x99,  0xf1,  0x16,  0x70,  0xb8, 
  0x63,  0x2a,  0x2c,  0x98,  0x16,  0x0a,  0x49,  0x22,  0x9c,  0x11,  0x30,  0x7b,  0x04,  0x04,  0x5c,  0x12, 
  0x90,  0x43,  0xb7,  0x70,  0x6a,  0xc5,  0xea,  0xea,  0x61,  0xa6,  0x73,  0xc4,  0x92,  0x09,  0xfa,  0x2b, 

  0x89,  0xad,  0x23,  0x60,  0xde,  0x0e,  0x5c,  0x39,  0x64,  0x10,  0x9e,  0x48,  0x46,  0xc0,  0x89,  0x29, 
  0x07,  0xce,  0x78,  0x11,  0xe1,  0x40,  0x54,  0x2d,  0xec,  0x18,  0xb2,  0x65,  0x8a,  0x8f,  0x30,  0x7b, 
  0x76,  0x9c,  0xb9,  0x86,  0x03,  0x93,  0x17,  0x39,  0xc0,  0x54,  0xde,  0xe4,  0xab,  0xa2,  0xc2,  0x82, 
  0x69,  0xa1,  0x90,  0x24,  0xc2,  0x19,  0x01,  0x33,  0xf6,  0x9c,  0x10,  0x70,  0x49,  0x40,  0xc6,  0xdd, 
  0xc2,  0xa9,  0x15,  0xab,  0xab,  0x87,  0x99,  0x72,  0x63,  0xca,  0x8e,  0x0b,  0x4b,  0x26,  0xe8,  0xaf, 
  0x24,  0xb6,  0xf0,  0xa0,  0x67,  0xce,  0x89,  0x3d,  0x57,  0x0e,  0x19,  0x18,  0x09,  0x9c,  0x99,  0x32, 
  0xff,  0x86,  0x72,  0x88,  0x86,  0x28,  0x60,  0xcc,  0x84,  0x65,  0x18,  0x00, 
};
*/


//defines from other library
#define MAC_PROTOCOL_ID 0x20

// define the operation codes, full byte

//notice that these conflict with the WIKI http://sourceforge.net/apps/trac/openlcb/wiki/BasicSpecDoc
#define MAC_CMD_WRITE                       0x00
#define MAC_CMD_READ                        0x40
#define MAC_CMD_OPERATION                   0x80
#define MAC_CMD_READ_REPLY                  0x50
#define MAC_CMD_GET_CONFIG_OPTIONS          0x80
#define MAC_CMD_GET_CONFIG_OPTIONS_REPLY    0x82
#define MAC_CMD_GET_ADD_SPACE_INFO          0x84
#define MAC_CMD_GET_ADD_SPACE_INFO_REPLY    0x86
#define MAC_CMD_LOCK                        0x88
#define MAC_CMD_LOCK_REPLY                  0x8A
#define MAC_CMD_GET_UNIQUEID                0x8C
#define MAC_CMD_GET_UNIQUEID_REPLY          0x8E

#define MAC_CMD_FREEZE                      0xA0
#define MAC_CMD_INDICATE                    0xA4
#define MAC_CMD_RESETS                      0xA8


#define MAC_CONFIG_OPTIONS_1_BYTE_WRITE      0x80
#define MAC_CONFIG_OPTIONS_2_BYTE_WRITE      0x40
#define MAC_CONFIG_OPTIONS_4_BYTE_WRITE      0x20
#define MAC_CONFIG_OPTIONS_64_BYTE_WRITE     0x10
#define MAC_CONFIG_OPTIONS_ARBITRARY_WRITE   0x02
#define MAC_CONFIG_OPTIONS_STREAM_WRITE      0x01

#define MAC_CONFIG_OPTIONS_WRITE_UNDER_MASK  0x80
#define MAC_CONFIG_OPTIONS_UNALIGNED_READS   0x40
#define MAC_CONFIG_OPTIONS_UNALIGNED_WRITES  0x20
#define MAC_CONFIG_OPTIONS_MFG_ACDI_FD_READ  0x08
#define MAC_CONFIG_OPTIONS_USR_ACDI_FC_READ  0x04
#define MAC_CONFIG_OPTIONS_USR_ACDI_FC_WRITE 0x02


/* Class to handle Memory Configuration protocol and CDI protocol */
class MyConfigHandler : 
public OLCB_Datagram_Handler
{
public:
  void create(OLCB_Link *link, OLCB_NodeID *nid, MyEventHandler *eventHandler);
//  void update(void);
  void datagramResult(bool accepted, uint16_t errorcode);
  void initialize(void);
  bool processDatagram(void);

  bool MACProcessRead(void);
  bool MACProcessWrite(void);
  bool MACProcessCommand(void);

private:
  MyEventHandler *_eventHandler; //for configuring PC-event memory space
  uint32_t getAddress(uint8_t* data);
  uint8_t decodeLength(uint8_t* data);
  uint8_t decodeSpace(uint8_t* data);
  uint8_t readCDI(uint16_t address, uint8_t length, uint8_t *data);
};


#endif

