#include <reset.h>

#include <ButtonLED.h>
#include <can.h>
#include <OpenLCB.h>

#include "DCC_Proxy.h"
//ButtonLed gold(GOLD);

//#define NUM_LOCOS 1
DCC_Proxy train; //[NUM_LOCOS];
OLCB_NodeID train_nid; //[NUM_LOCOS];
OLCB_CAN_Link link;

void setup()
{
	Serial.begin(115200);
	
	Serial.println("Roster Node begin!");

	
	link.initialize();
	train_nid.set(2, 1, 13, 42, 23, 0); //DIY address space
	train.create(&link, &train_nid);
	train.setDCCAddress(3); //hardcoded. short address; notice that this method takes an address encoded as per RP9.2.1
	link.addVNode(&train);
	
//	gold.on(0xFFFFFFFEL); // unready blink until intialized

	Serial.println("Initialization complete!");
}

void loop()
{
	link.update();
	//gold.process();
}
