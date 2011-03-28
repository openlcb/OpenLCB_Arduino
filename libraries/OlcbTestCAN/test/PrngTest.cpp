//===========================================================
// CanMrrlcbTest
//   Developing (eventual) classes for OpenLCB
// 
//   Bob Jacobsen 2009
//      based on examples by Alex Shepherd and David Harris
//===========================================================
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

// The following line is needed because the Arduino environment 
// won't search a library directory unless the library is included 
// from the top level file (this file)
#include <CAN.h>
#include "logging.h"

// OpenLCB definitions
#include "OpenLcbCanInterface.h"
#include "OpenLcbCanBuffer.h"
#include "NodeID.h"
#include "EventID.h"

// specific OpenLCB implementations
#include "LinkControl.h"

OpenLcbCanBuffer     rxBuffer;	// CAN receive buffer
OpenLcbCanBuffer     txBuffer;	// CAN send buffer
OpenLcbCanBuffer*    ptxCAN;

NodeID n0(0,0,0,0,0,0);
NodeID n1(02,01,0x21,00,00,0x12);
NodeID n2(02,01,0x12,00,00,0x21);
NodeID n3(02,01,0x11,00,00,0x22);
NodeID n4(02,01,0x22,00,00,0x11);

OpenLcbCanBuffer b;

class TestLinkControl : public LinkControl {
    public:
    TestLinkControl(OpenLcbCanBuffer* txb, NodeID* n) : LinkControl(txb,n) {}
    
    void nextAlias() {
        LinkControl::nextAlias();
    }
    void dump() {
        printf(" %X %X -> %X \n", lfsr1, lfsr2, getAlias());
    }
    void setSeed(long l1, long l2) {
        lfsr1 = l1;
        lfsr2 = l2;
    }
};

int main( int argc, const char* argv[] )
{
    {
        printf("--------------\n");
        TestLinkControl link1(&txBuffer, &n1);
        link1.reset();

        link1.setSeed(0,0);
        link1.dump();
        link1.nextAlias();
        link1.dump();
        link1.nextAlias();
        link1.dump();
        link1.nextAlias();
        link1.dump();
        link1.nextAlias();
        link1.dump();
    }

    {
        printf("--------------\n");
        TestLinkControl link1(&txBuffer, &n1);
        link1.reset();

        link1.setSeed(0x020121,0x000012);
        link1.dump();
        link1.setSeed(0x020112,0x000021);
        link1.dump();
        link1.setSeed(0x020111,0x000022);
        link1.dump();
        link1.setSeed(0x020122,0x000011);
        link1.dump();
    }
}

