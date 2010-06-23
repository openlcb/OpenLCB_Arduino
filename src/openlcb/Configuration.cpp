// makes this an Arduino file
#include "WConstants.h"

#include "Configuration.h"
#include "Datagram.h"
#include "Stream.h"

#include "logging.h"


#define CONFIGURATION_DATAGRAM_CODE 0x23

// define the operation codes, full byte
#define CFG_CMD_WRITE                       0x00
#define CFG_CMD_READ                        0x40
#define CFG_CMD_OPERATION                   0x80

#define CFG_CMD_READ_REPLY                  0x50

#define CFG_CMD_GET_CONFIG                  0x80
#define CFG_CMD_GET_CONFIG_REPLY            0x82
#define CFG_CMD_GET_ADD_SPACE_INFO          0x84
#define CFG_CMD_GET_ADD_SPACE_INFO_REPLY    0x86
#define CFG_CMD_LOCK                        0x88
#define CFG_CMD_LOCK_REPLY                  0x8A
#define CFG_CMD_GET_UNIQUEID                0x8C
#define CFG_CMD_GET_UNIQUEID_REPLY          0x8E

#define CFG_CMD_FREEZE                      0xA0
#define CFG_CMD_INDICATE                    0xA4
#define CFG_CMD_RESET                       0xA8
#define CFG_CMD_FACTORY_RESET               0xAC

/** 
 * Structure: Requests come in via 
 * receiveDatagram.  Results (if any) are stored in a 
 * buffer, and check() then sends it when possible.
 *
 * TODO: No stream implementation yet.
 * TODO: Uses an extra buffer, which it would be good to avoid,
 *       but I've included it now to not make assumptions about
 *       Datagram use & structure
 */
 
Configuration::Configuration(Datagram* d, Stream *s) {
    dg = d;
    str = s;
    request = false;
}

void Configuration::check() {
    if (!request) return;
    // have a request pending
    switch (buffer[1]&0xC0) {
        case CFG_CMD_READ:
            processRead(buffer, length);
            break;
        case CFG_CMD_WRITE:
            processWrite(buffer, length);
            break;
        case CFG_CMD_OPERATION:
            processCmd(buffer, length);
            break;
    }
}

int Configuration::receivedDatagram(uint8_t* data, int ln) {
    // decode whether this is a configuration request
    if (data[0] != CONFIGURATION_DATAGRAM_CODE) return 0;
    // yes, copy to our buffer
    length = ln;
    for (int i = 0; i<length; i++) 
        buffer[i] = *(data++);
    // mark as ready
    request = true;
    return 0;
}

uint32_t Configuration::getAddress(uint8_t* data) {
    uint32_t val = 0;
    val |= (data[2]<<24);
    val |= (data[3]<<16);
    val |= (data[4]<<8);
    val |= (data[5]);
    return val;
}

// -1 means stream
int Configuration::decodeLen(uint8_t* data) {
    int count = data[1]&0x7;
    int val;
    if ( count <= 6) val = 1 << count;
    else val = -1;
    
    return val;
}
int Configuration::decodeSpace(uint8_t* data) {
    int val;
    switch (data[1]&0x18) {
        case 0x00:
            val = 0xFF;
            break;
        case 0x08:
            val = 0xFE;
            break;
        case 0x10:
            val = 0xFD;
            break;
        case 0x18:
            val = data[6];
            break;
    }
    return val;
}

void Configuration::processRead(uint8_t* data, int length) {
    printf("  processRead start=0x%x len=%d space=%x\n", getAddress(data), decodeLen(data), decodeSpace(data) );
    // see if we can get datagram buffer to reply
    uint8_t* d = dg->getTransmitBuffer();
    if (d == 0) return; // skip and return again later
    // will reply, mark as done.
    request = false;
    d[0]=0xAA; d[1]=0xBB;
    dg->sendTransmitBuffer(2, 0x123);
}

void Configuration::processWrite(uint8_t* data, int length) {
    printf("  processWrite start=0x%x space=%x\n", getAddress(data), decodeSpace(data) );
}

void Configuration::processCmd(uint8_t* data, int length) {
    printf("  processCmd\n");
}

