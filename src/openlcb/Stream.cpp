// makes this an Arduino file
#include "WConstants.h"

#include "OpenLcbCan.h"
#include "OpenLcbCanBuffer.h"
#include "LinkControl.h"
#include "Stream.h"

#include "logging.h"

// ToDo: Implementation incomplete

Stream::Stream(OpenLcbCanBuffer* b, unsigned int (*cb)(uint8_t *tbuf, unsigned int length), LinkControl* ln) {
}

void Stream::check() {
    // see if can send.
}

void Stream::receivedFrame(OpenLcbCanBuffer* rcv) {
    // check for init stream
    // check for stream data transfer headed here
    
    // check for init stream reply
    // check for stream ack to send more
}

