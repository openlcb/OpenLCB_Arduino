// makes this an Arduino file
#include "WConstants.h"
#include "stdio.h"

//#include "OpenLcbCanInterface.h"
#include "OpenLcbCanBuffer.h"

void printBuffer(OpenLcbCanBuffer* b) {
    printf("[%08x]", b->id);
    for (int i = 0 ; i<b->length; i++) {
        printf(" %02x", b->data[i]);
    }
}

void OpenLcb_can_init() {
    printf("OpenLcb_can_init\n");
}

// Can a (the) CAN buffer be used?  
// Generally, indicates the buffer can be immediately
// queued for transmit, so it make sense to prepare it now
bool OpenLcb_can_xmt_ready(OpenLcbCanBuffer* b) {
    //printf("OpenLcb_can_xmt_ready: ");
    //printBuffer(b);
    //printf("\n");
    return true;   // at least one has space
}

// Queue a CAN frame for sending, if possible
// Returns true if queued, false if not currently possible
bool OpenLcb_can_queue_xmt_immediate(OpenLcbCanBuffer* b) {
    printf("queue_xmt_immediate: ");
    printBuffer(b);
    printf("\n");
    return true;
}

// Queue a CAN frame for sending; spins until it can queue
void OpenLcb_can_queue_xmt_wait(OpenLcbCanBuffer* b) {
    printf("queue_xmt_wait: ");
    printBuffer(b);
    printf("\n");
}

// Send a CAN frame, waiting until it has been sent
void OpenLcb_can_send_xmt(OpenLcbCanBuffer* b) {
    printf("can_send_xmt: ");
    printBuffer(b);
    printf("\n");
}

// Check whether all frames have been sent,
// a proxy for the link having gone idle
bool OpenLcb_can_xmt_idle() {
    printf("OpenLcb_can_xmt_idle\n");
    return true;   // All empty, nothing to send
}

OpenLcbCanBuffer* queuedBuffer = 0;

// Make the oldest received CAN frame available,
// in the process removing it from the CAN subsystem.
// Return false (zero) if no frame available.
bool OpenLcb_can_get_frame(OpenLcbCanBuffer* b) {
    if (queuedBuffer == 0) {
        //printf("OpenLcb_can_get_frame: nothing available\n");
        return false;
    } else {
        b->id = queuedBuffer->id;
        b->length = queuedBuffer->length;
        int n = b->length;
        for (int i = 0; i < n; i++) {
            (b->data[i]) = (queuedBuffer->data[i]);
        }
        queuedBuffer = 0;
        printf("can_get_frame:  ");
        printBuffer(b);
        printf("\n");
        return  true;
    }
}

void queueTestMessage(OpenLcbCanBuffer* b) {
    queuedBuffer = b;
}
