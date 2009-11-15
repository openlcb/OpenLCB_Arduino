/**
 * A work in progress, from an email by
 * dpharris November 2009
 */

// Pseudo-code demonstrating use:
  
  
oLCB_buff_type serialBuf;
serialOpen(0, 9600); // optional baud rate
serialOpen(1, 19200); // optional baud rate
while(!serialReceive(0, &serialBuf)) { // receive on channel 0
    if(serialBuf.nDataBytes==1 && serialBuf.data[0]=='$') break;
    while(!serialSend(1, &serialBuf, 10){};
    // wait forever to send on channel 1
    // trying again every 10 ms
}
serialClose(0);
serialClose(1);


oLCB_buff_type oLCBbuf;
canOpen(0, 125000);
serialOpen(0, 9600);
while(1==1) {
if(canReceive(0, oLCBbuf)) {
    if(!serialSend(0, oLCBbuf, 10)) error("Lost packet on serial");
}
if(serialReceive(&oLCBbuf)) {
    if(!canSend(0, oLCBbuf, 1)) error("Lost packet on CAN");
}
canClose(0);
serialClose(0);

FIFO Buffering:

int nFifoBuffs = 10;
oLCB_fifo_type fifoBuffs[nFifoBuffs];
first = 0;
last =0;
while(1==1) {
    if(canReceive(0, fifoBuffs[last])) {
        if( (last+=1)>nFifoBuffs) {
            last=0;
            if(last==first) error("Fifo overrun");
        }
        // try to send
        if(first!=last) { // if the fifo is not empty, and
            if(buffs[first].status==WAITING) // first isn't being sent already, then
                canSend(1, fifoBuffs[first], 10); // send it, and give it 10 ms
        if(fifoBuffs[first].status==SENT) { // if first is sent then remove from fifo
            if( (first+=1)>nFifoBuffs) first=0;
        }
    }
}
