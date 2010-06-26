// makes this an Arduino file
#include "WConstants.h"

#include "Configuration.h"
#include "Datagram.h"
#include "Stream.h"

#include "logging.h"


#define CONFIGURATION_DATAGRAM_CODE 0x20

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
 
  Configuration::Configuration( Datagram *d, Stream *s,
                        const uint8_t* (*gr)(int address, int space),
                        uint8_t* (*gw)(int address, int space),
                        void (*res)()
                 ){
    dg = d;
    str = s;
    request = false;
    getWrite = gw;
    getRead = gr;
    restart = res;
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

int Configuration::receivedDatagram(uint8_t* data, int ln, unsigned int f) {
    // decode whether this is a configuration request
    if (data[0] != CONFIGURATION_DATAGRAM_CODE) return 0;
    // yes, copy to our buffer
    length = ln;
    from = f;
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
    //logstr("  processReadp\n");
    // see if we can get datagram buffer to reply
    uint8_t* d = dg->getTransmitBuffer();
    if (d == 0) return; // skip and return again later
    // will reply, mark as done.
    request = false;
    // copy front matter
    for (int i=0; i<6; i++)
        d[i]=data[i];
    d[0] = CFG_CMD_READ_REPLY | d[0]&0x0F;
    // get length, space
    int len = decodeLen(data);
    int space = decodeSpace(data);
    // TODO: copy real data
    // TODO: from address spaces
    for (int i=0; i<len; i++)
        d[i+6] = i+16;
    // send
    dg->sendTransmitBuffer(6+len, from);
}

void Configuration::processWrite(uint8_t* data, int length) {
    //logstr("  processWrite\n");
    // TODO: Copy data into place
    // TODO: with proper address space
}

void Configuration::processCmd(uint8_t* data, int length) {
    //logstr("  processCmd\n");
    switch (data[1]&0xFC) {
        case CFG_CMD_GET_CONFIG: {  // to partition local variable below
            // reply with canned message
            uint8_t* d = dg->getTransmitBuffer();
            if (d==0) return; // skip and return again later
            // will reply, mark as done.
            request = false;
            d[0]=CONFIGURATION_DATAGRAM_CODE; d[1]=CFG_CMD_GET_CONFIG_REPLY;
            d[2]=0x03;d[3]=0x01;d[4]=0x38;d[5]=0x00;d[6]=0x00;
            dg->sendTransmitBuffer(7, from);
            break;
          }
        case CFG_CMD_GET_ADD_SPACE_INFO: {  // to partition local variable below
            // reply with canned message
            uint8_t* d = dg->getTransmitBuffer();
            if (d==0) return; // skip and return again later
            // will reply, mark as done.
            request = false;
            d[0]=CONFIGURATION_DATAGRAM_CODE; d[1]=CFG_CMD_GET_CONFIG_REPLY;
            d[2]=0x03;d[3]=0x01;d[4]=0x38;d[5]=0x00;d[6]=0x00;
            dg->sendTransmitBuffer(7, from);
            break;
          }
        //case CFG_CMD_CFG_CMD_GET_CONFIG_REPLY :
        //case CFG_CMD_CFG_CMD_GET_ADD_SPACE_INFO_REPLY:
        //case CFG_CMD_LOCK:
        //case CFG_CMD_LOCK_REPLY:
        //case CFG_CMD_GET_UNIQUEID:
        //case CFG_CMD_GET_UNIQUEID_REPLY:
        //case CFG_CMD_FREEZE:
        //case CFG_CMD_INDICATE:
        //case CFG_CMD_RESET:
        //case CFG_CMD_FACTORY_RESET:
        //case CFG_CMD_INDICATE:
        default:
            // these do nothing in this implementation
            request = false;
            break;
    }
}
