/************************
AdvancedDCS
A sketch to aid the development of the OpenLCB Traction and related Protocols
Copyright (C)2012 Railstars Limited

This file is part of AdvancedDCS.

    AdvancedDCS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AdvancedDCS.  If not, see <http://www.gnu.org/licenses/>.
************************/

#ifndef __TRAIN_H__
#define __TRAIN_H__

#include "OpenLCB.h"
#include "DCCPacketScheduler.h"

#define RELEASE_TIMEOUT  900000 //15 minutes.
#define REFRESH_TIMEOUT  5000 //5 seconds

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

class Train : public OLCB_Datagram_Handler
{  
  public:
    void initialize(void);
    void create(OLCB_Link *link, OLCB_NodeID *nid, DCCPacketScheduler *dcs);    
    void update(void);
    void datagramResult(bool accepted, uint16_t errorcode);
    uint16_t processDatagram(void);
    bool verified;
    uint16_t getAddress(void) {return _address;}
  private:
    uint32_t getAddress(uint8_t* data);
    uint8_t decodeLength(uint8_t* data);
    uint8_t decodeSpace(uint8_t* data);
    uint16_t MACProcessWrite(void);
    void updateSpeed(void);
    DCCPacketScheduler *_dcs;
    int8_t _speed;
    uint8_t _speed_steps;
    uint16_t _address;
    uint8_t _address_kind;
    uint32_t _FX;
    uint32_t _refresh_timer, _release_timer;
};

#endif
