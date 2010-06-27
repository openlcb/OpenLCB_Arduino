//===========================================================
// StreamTest
//   Tests for OpenLCB Stream support
// 
//   Bob Jacobsen 2010 
//===========================================================
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "logging.h"
#include "NodeMemory.h"

#include "EEPROM.h"
#include "Event.h"
#include "NodeID.h"

NodeID node(1,2,3,4,5,6);

Event cEvents[] = {
    Event(1,2,3,4,5,6,7,8), 
    Event(17,18,19,20,21,22,23,24)
};
int cEventNum = 2;

Event pEvents[] = {
    Event(33,34,35,36,37,38,39,40), 
    Event(65,66,67,68,69,70,71,72)
};
int pEventNum = 2;

NodeMemory m;

int main( int argc, const char* argv[] )
{
    printf("do setup\n");
    m.setup(0, &node, cEvents, cEventNum, pEvents, cEventNum);
    EEPROM.dump();

    printf("do setup\n");
    m.setup(0, &node, cEvents, cEventNum, pEvents, cEventNum);
    EEPROM.dump();

    printf("do reset\n");
    m.reset(0, &node, cEvents, cEventNum, pEvents, cEventNum);
    EEPROM.dump();

    printf("do reset\n");
    m.reset(0, &node, cEvents, cEventNum, pEvents, cEventNum);
    EEPROM.dump();

    printf("do setup\n");
    m.setup(0, &node, cEvents, cEventNum, pEvents, cEventNum);
    EEPROM.dump();

    printf("change values\n");
    cEvents[0] = pEvents[1];
    m.store(0, &node, cEvents, cEventNum, pEvents, cEventNum);
    EEPROM.dump();

    printf("change values\n");
    Event a(10,11,12,13,14,15,16,17);
    cEvents[1] = a;
    m.store(0, &node, cEvents, cEventNum, pEvents, cEventNum);
    EEPROM.dump();
    
}