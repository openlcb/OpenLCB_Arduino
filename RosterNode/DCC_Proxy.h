// #ifdef DO_OLCB_DCC_TRAIN

#ifndef __DCC_PROXY_H__
#define __DCC_PROXY_H__

#include "OLCB_Datagram_Handler.h"
#include "OLCB_Datagram.h"

#define LONG_ADDRESS 1
#define SHORT_ADDRESS 0

#define DATAGRAM_MOTIVE             	    0x30
#define DATAGRAM_MOTIVE_SETSPEED			0x01
#define DATAGRAM_MOTIVE_GETSPEED			0x03
#define DATAGRAM_MOTIVE_REPORTSPEED			0x02

#define SPEED_STEPS_14	14
#define SPEED_STEPS_28	28
#define SPEED_STEPS_128	128

// #define NUM_SIMULTANEOUS_CONTROLLERS 2

//XML CDI data, taken from Io_8P_8C_default.xml
const static PROGMEM char cdixml[] = "<?xml version='1.0' encoding='UTF-8'?><?xml-stylesheet type='text/xsl' href='xslt/cdi.xsl'?><cdi xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='http://openlcb.org/trunk/prototypes/xml/schema/cdi.xsd'><identification><manufacturer>Railstars Limited</manufacturer><model>Roster Demo</model><hardwareVersion>$.%</hardwareVersion><softwareVersion>1.0</softwareVersion></identification><acdi /><segment><group><name>User Identification</name><description>Add your own descriptive name and description for this node.</description><string size='32'><name>Node Name</name></string><string size='64'><name>Node Description</name></string></group></segment><segment><group><name>Fun Stuff</name><int size='2'><name>DCC Address</name></int><int size='1'><name>DCC Address Kind</name><map><relation><property>0</property><value>Short Address</value></relation><relation><property>1</property><value>Long Address</value></relation></map></int><int size='1'><name>Speed Step Selection</name><map><relation><property>1</property><value>14 Steps</value></relation><relation><property>2</property><value>28 Steps</value></relation><relation><property>3</property><value>128 Steps</value></relation></map></int></group></segment></cdi>";


//defines from other library
#define DATAGRAM_MEMCONFIG 0x20

// define the operation codes, full byte

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

class DCC_Proxy :
public OLCB_Datagram_Handler
{
	public:
	void create(OLCB_Link *link, OLCB_NodeID *nid);
	void update(void);
	uint16_t processDatagram(OLCB_Datagram *datagram);
	void datagramResult(bool accepted, uint16_t errorcode);

//bool DCC_Proxy_isAttached(OLCB_NodeID *node) {return true;} //no security;
void setDCCAddress(uint16_t new_address);

uint16_t MACProcessRead(void);
uint16_t MACProcessWrite(void);
uint16_t MACProcessCommand(void);

uint32_t getAddress(uint8_t* data);
uint8_t decodeLength(uint8_t* data);
uint8_t decodeSpace(uint8_t* data);
uint8_t readCDI(uint16_t address, uint8_t length, uint8_t *data);

private:
uint8_t _metersPerSecondToDCCSpeed(float mps);

uint16_t _handleTractionDatagram(OLCB_Datagram *datagram);
uint16_t _handleMemConfigDatagram(OLCB_Datagram *datagram);

//TODO need to do these!
	 // uint16_t handleAttachDatagram(OLCB_Datagram *datagram) {return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;}
	 // uint16_t handleReleaseDatagram(OLCB_Datagram *datagram) {return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;}
	  uint16_t _handleSetSpeedDatagram(OLCB_Datagram *datagram);
	  uint16_t _handleGetSpeedDatagram(OLCB_Datagram *datagram);

	 // uint16_t handleSetFXDatagram(OLCB_Datagram *datagram) {return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;}
	 // uint16_t handleGetFXDatagram(OLCB_Datagram *datagram) {return DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED;}
  
  
  //helpers
    uint32_t _timer;
//    OLCB_Datagram *DCC_Proxy_txDatagramBuffer;
//    OLCB_Datagram *DCC_Proxy_rxDatagramBuffer;
//    OLCB_Link *DCC_Proxy_link;

  //configuration
    uint16_t _dcc_address;
    //uint8_t _dcc_address_kind;
    uint8_t _speed_curve[128];

  //state infor
  	int8_t _speed; //in speedsteps; signed for direction
 //   uint32_t DCC_Proxy_FX; //bitfield of 32 FX
 //   OLCB_NodeID *DCC_Proxy_controllers[NUM_SIMULTANEOUS_CONTROLLERS];
};


#endif //__DCC_PROXY_H__

