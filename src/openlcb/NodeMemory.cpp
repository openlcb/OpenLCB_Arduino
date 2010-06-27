// makes this an Arduino file
#include "WConstants.h"

#include "NodeMemory.h"

#include "logging.h"
#include "EventID.h"
#include "Event.h"
#include "NodeID.h"
#include <EEPROM.h>

// ToDo: NodeID* not kept in object member to save RAM space, may be false economy

NodeMemory::NodeMemory(int start) {
    startAddress = start;
    count = 0;
}

void NodeMemory::setup(NodeID* nid, Event* cE, int nC, Event* pE, int nP) {
    if (!checkOK()) {
        // have to reload
        // clear the count
        writeByte(startAddress+4, 0);
        writeByte(startAddress+5, 0);
        // handle the rest
        reset(nid, cE, nC, pE, nP);
    }
    // read NodeID from non-volative memory
    uint8_t* p;
    int addr = startAddress+6; // skip check word and count
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

void NodeMemory::reset(NodeID* nid, Event* cE, int nC, Event* pE, int nP) {
    // Do the in-memory update. Does not change
    // the total count, this is not an "initial config" for factory use.

    Event* c;
    c = cE;
    for (int i = 0; i<nC; i++) {
        setToNewEventID(nid, c++);
    }
    
    c = pE;
    for (int i = 0; i<nP; i++) {
        setToNewEventID(nid, c++);
    }
    // and store
    store(nid, cE, nC, pE, nP);
}

void NodeMemory::store(NodeID* nid, Event* cE, int nC, Event* pE, int nP) {
    
    int addr = startAddress;
    // write tag
    writeByte(addr++, 0xEE);
    writeByte(addr++, 0x55);
    writeByte(addr++, 0x5E);
    writeByte(addr++, 0xE5);

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

void NodeMemory::setToNewEventID(NodeID* nid, EventID* eventID) {
    uint8_t* p = (uint8_t*)eventID;
    uint8_t* n = (uint8_t*)nid;
    for (int k=0; k<sizeof(*nid); k++)
        *p++ = *n++;
    *p++ = (count++>>8)&0xFF;
    *p++ = count&0xFF;
}

bool NodeMemory::checkOK() {
    if (EEPROM.read(startAddress  ) != 0xEE ) return false;
    if (EEPROM.read(startAddress+1) != 0x55 ) return false;
    if (EEPROM.read(startAddress+2) != 0x5E ) return false;
    if (EEPROM.read(startAddress+3) != 0xE5 ) return false;
    return true;
}

// write only if needed
void NodeMemory::writeByte(int addr, uint8_t b) {
    if (EEPROM.read(addr) != b ) {
        EEPROM.write(addr, b);
    }
}