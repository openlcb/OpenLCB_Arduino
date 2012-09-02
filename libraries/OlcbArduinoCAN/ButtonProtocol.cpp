


#include "ButtonProtocol.h"

#include <string.h>


static LinkControl* link;
static OpenLcbCanBuffer* buffer;
static uint16_t dest;
static bool queued;


#define MTI_BP_REQUEST 0x948
#define MTI_BP_REPLY   0x949

ButtonProtocol::ButtonProtocol(ButtonLed** bC, uint8_t n, ButtonLed* bptr, ButtonLed* gptr) {
	buttons = bC;
	nButtons = n;
	blue = bptr;
	gold = gptr;
}
  

void ButtonProtocol::check() {
	logstr("\n   4.1");
    if (queued) {
		//logstr(",4.2");
		if (OpenLcb_can_xmt_ready(buffer)) {
			logstr(",4.3");
			//buffer->setOpenLcbMTI(MTI_BP_REPLY);                  
			//buffer->setDestAlias(dest);
			// pack the blue, gold, and buttons states into an bit-array
			uint8_t nBytes = (2+nButtons)/8+1;
			if(nBytes>8) nBytes = 8;
			indicators[0]  = ( (blue->pattern & 0x1) ? 1 : 0 );
			indicators[0] |= ( (gold->pattern & 0x1) ? 1 : 0 )<<1;
			//logstr("\n nB="); logstr((uint8_t)nButtons);
			for(uint8_t i=2; i<(2+nButtons); i++) {
				bit= i % 8;
				byt= i / 8;
				indicators[byt] |= ( (buttons[i-2]->pattern & 0x1) ? 1 : 0 )<<bit;
				logstr("\n indicators="); logstr((uint8_t)indicators[byt]);
			}
			//memcpy( &(buffer->data[2]), indicators, nBytes) ; 
			//buffer->length = 2+nBytes;
			//OpenLcb_can_queue_xmt_immediate(buffer);  // checked previously
			queued = false;
			logstr(",4.4");
		}
	}
}

bool ButtonProtocol::receivedFrame(OpenLcbCanBuffer* rcv) {
    if ( rcv->isOpenLcbMTI(MTI_BP_REQUEST) )  { 
        // OK, start process
		logstr("\n   3.1");
//		blue->state = ( (rcv->data[2]&0x01 ? true : false) );
//		gold->state = ( (rcv->data[2]&0x01 ? true : false) );
		blue->updateState( (rcv->data[2]&0x01 ? true : false) );
		gold->updateState( (rcv->data[2]&0x02 ? true : false) );
		//logstr("\n   3.2");
		uint8_t button=0;
		while(button<nButtons) {
			logstr(",");logstr(button);
			byt = (button+2) / 8;
			bit = (button+2) % 8;
			logstr(" "); logstr(rcv->data[byt+2]);
			if( (rcv->data[byt+2] & (1<<bit)) ) {
//				buttons[button]->state = true;
				buttons[button]->setVirtualState(true);
				logstr(" on");
			} else {
//				buttons[button]->state = false;
				buttons[button]->setVirtualState(false);
				logstr(" off");
			} 
			button++;
		}
		queued = true;
		dest = rcv->getSourceAlias();
		logstr(",3.2");
        return true;
    }
    return false;
}

void ButtonProtocol::sendUI(char* s) {
    buffer->setOpenLcbMTI(MTI_BP_REPLY);
    buffer->length = strlen(s);
    memcpy(buffer->data, s, strlen(s));
	OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
}