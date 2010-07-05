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
    Event(17,18,19,20,21,22,23,24),
    Event(33,34,35,36,37,38,39,40), 
    Event(65,66,67,68,69,70,71,72)
};
int cEventNum = 4;

NodeMemory m(0);

int main( int argc, const char* argv[] )
{
    printf("do setup\n");
    m.setup(&node, cEvents, cEventNum);
    EEPROM.dump();

    printf("do setup\n");
    m.setup(&node, cEvents, cEventNum);
    EEPROM.dump();

    printf("do reset\n");
    m.reset(&node, cEvents, cEventNum);
    EEPROM.dump();

    printf("do reset\n");
    m.reset(&node, cEvents, cEventNum);
    EEPROM.dump();

    printf("do setup\n");
    m.setup(&node, cEvents, cEventNum);
    EEPROM.dump();

    printf("change values\n");
    cEvents[0] = cEvents[3];
    m.store(&node, cEvents, cEventNum);
    EEPROM.dump();

    printf("change values\n");
    Event a(10,11,12,13,14,15,16,17);
    cEvents[1] = a;
    m.store(&node, cEvents, cEventNum);
    EEPROM.dump();
    
}