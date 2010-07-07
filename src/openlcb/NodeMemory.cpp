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

void NodeMemory::forceInitAll() {
    EEPROM.write(0,0xFF);
    EEPROM.write(1,0xFF);
}

void NodeMemory::forceInitEvents() {
    EEPROM.write(2,0x33);
    EEPROM.write(3,0xCC);
}

void NodeMemory::setup(NodeID* nid, Event* cE, int nC) {
    if (checkNidOK()) {
        // read NodeID from non-volative memory
        uint8_t* p;
        int addr = startAddress+6; // skip check word and count
        p = (uint8_t*)nid;
        for (int i=0; i<sizeof(NodeID); i++) 
        *p++ = EEPROM.read(addr++);

        // load count
        int part1 = EEPROM.read(startAddress+4);
        int part2 = EEPROM.read(startAddress+5);
        count = (part1<<8)+part2;

        // handle the rest
        reset(nid, cE, nC);

    } else if (!checkAllOK()) {
        count = 0;

        // handle the rest
        reset(nid, cE, nC);
    }
    
    // read NodeID from non-volative memory
    uint8_t* p;
    int addr = startAddress+6; // skip check word and count
    p = (uint8_t*)nid;
    for (int i=0; i<sizeof(NodeID); i++) 
        *p++ = EEPROM.read(addr++);

    
    // read events
    p = (uint8_t*)cE;
    for (int k=0; k<nC; k++)
        for (int i=0; i<sizeof(Event); i++) 
            *p++ = EEPROM.read(addr++);
    
}

void NodeMemory::reset(NodeID* nid, Event* cE, int nC) {
    // Do the in-memory update. Does not change
    // the total count, this is not an "initial config" for factory use.

    Event* c;
    c = cE;
    for (int i = 0; i<nC; i++) {
        setToNewEventID(nid, c++);
    }
    
    // and store
    store(nid, cE, nC);
}

void NodeMemory::store(NodeID* nid, Event* cE, int nC) {
    
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

    // write events
    p = (uint8_t*)cE;
    for (int k=0; k<nC; k++) {
        for (int i=0; i<sizeof(EventID); i++) 
            writeByte(addr++, *p++);
        for (int i=sizeof(EventID); i<sizeof(Event); i++) {
            // skip over the flags
            writeByte(addr++, 0);
            p++;
         }
    }
}

void NodeMemory::setToNewEventID(NodeID* nid, EventID* eventID) {
    uint8_t* p = (uint8_t*)eventID;
    uint8_t* n = (uint8_t*)nid;
    for (int k=0; k<sizeof(*nid); k++)
        *p++ = *n++;
    *p++ = (count++>>8)&0xFF;
    *p++ = count&0xFF;
}

bool NodeMemory::checkAllOK() {
    if (EEPROM.read(startAddress  ) != 0xEE ) return false;
    if (EEPROM.read(startAddress+1) != 0x55 ) return false;
    if (EEPROM.read(startAddress+2) != 0x5E ) return false;
    if (EEPROM.read(startAddress+3) != 0xE5 ) return false;
    return true;
}
bool NodeMemory::checkNidOK() {
    if (EEPROM.read(startAddress  ) != 0xEE ) return false;
    if (EEPROM.read(startAddress+1) != 0x55 ) return false;
    if (EEPROM.read(startAddress+2) != 0x33 ) return false;
    if (EEPROM.read(startAddress+3) != 0xCC ) return false;
    return true;
}

// write only if needed
void NodeMemory::writeByte(int addr, uint8_t b) {
    if (EEPROM.read(addr) != b ) {
        EEPROM.write(addr, b);
    }
}