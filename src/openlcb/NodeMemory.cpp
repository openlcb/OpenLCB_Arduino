// makes this an Arduino file
#include "WConstants.h"

#include "NodeMemory.h"

#include "logging.h"
#include "Event.h"
#include "NodeID.h"
#include "EEPROM.h"

// doesn't do anything
NodeMemory::NodeMemory() {}

void NodeMemory::setup(int addr, NodeID* nid, Event* cE, int nC, Event* pE, int nP) {
    if (!checkOK(addr)) {
        // have to reload
        // clear the count
        writeByte(addr+4, 0);
        writeByte(addr+5, 0);
        // handle the rest
        reset(addr, nid, cE, nC, pE, nP);
    }
    // read NodeID from non-volative memory
    uint8_t* p;
    addr = addr+6; // skip check word and count
    p = (uint8_t*)nid;
    for (int i=0; i<sizeof(NodeID); i++) 
        *p++ = EEPROM.read(addr++);

    // read consumer events
    p = (uint8_t*)cE;
    for (int k=0; k<nC; k++)
        for (int i=0; i<sizeof(Event); i++) 
            *p++ = EEPROM.read(addr++);

    // read consumer events
    p = (uint8_t*)pE;
    for (int k=0; k<nP; k++)
        for (int i=0; i<sizeof(Event); i++) 
            *p++ = EEPROM.read(addr++);
    
}

void NodeMemory::reset(int addr, NodeID* nid, Event* cE, int nC, Event* pE, int nP) {
    // do the in-memory update
    // TODO: Not updating the count for used events properly, using
    // TODO: the over-all store count, which includes writing back event changes
    int part1 = EEPROM.read(addr+4);
    int part2 = EEPROM.read(addr+5);
    uint16_t count = ((part1<<8)+part2)*(nP+nC)+1;
    Event* c;
    
    c = cE;
    for (int i = 0; i<nC; i++) {
        uint8_t* p = (uint8_t*)c++;
        uint8_t* n = (uint8_t*)nid;
        for (int k=0; k<sizeof(*nid); k++)
            *p++ = *n++;
        *p++ = (count>>8)&0xFF;
        *p++ = count&0xFF;
        count++;
    }
    
    c = pE;
    for (int i = 0; i<nP; i++) {
        uint8_t* p = (uint8_t*)c++;
        uint8_t* n = (uint8_t*)nid;
        for (int k=0; k<sizeof(*nid); k++)
            *p++ = *n++;
        *p++ = (count>>8)&0xFF;
        *p++ = count&0xFF;
        count++;
    }
    // and store
    store(addr, nid, cE, nC, pE, nP);
}

void NodeMemory::store(int addr, NodeID* nid, Event* cE, int nC, Event* pE, int nP) {
    // write tag
    writeByte(addr++, 0xEE);
    writeByte(addr++, 0x55);
    writeByte(addr++, 0x5E);
    writeByte(addr++, 0xE5);

    // TODO: is this right way to handle count?
    // incremement count of writes
    int part1 = EEPROM.read(addr);
    int part2 = EEPROM.read(addr+1);
    uint16_t count = (part1<<8)+part2;
    count++;
    writeByte(addr++, (count>>8)&0xFF);
    writeByte(addr++, (count)&0xFF);
    
    // write NodeID
    uint8_t* p;
    p = (uint8_t*)nid;
    for (int i=0; i<sizeof(NodeID); i++) 
        writeByte(addr++, *p++);

    // write consumer events
    p = (uint8_t*)cE;
    for (int k=0; k<nC; k++)
        for (int i=0; i<sizeof(Event); i++) 
            writeByte(addr++, *p++);

    // write consumer events
    p = (uint8_t*)pE;
    for (int k=0; k<nP; k++)
        for (int i=0; i<sizeof(Event); i++) 
            writeByte(addr++, *p++);
}

bool NodeMemory::checkOK(int addr) {
    if (EEPROM.read(addr  ) != 0xEE ) return false;
    if (EEPROM.read(addr+1) != 0x55 ) return false;
    if (EEPROM.read(addr+2) != 0x5E ) return false;
    if (EEPROM.read(addr+3) != 0xE5 ) return false;
    return true;
}

// write only if needed
void NodeMemory::writeByte(int addr, uint8_t b) {
    if (EEPROM.read(addr) != b ) {
        EEPROM.write(addr, b);
    }
}