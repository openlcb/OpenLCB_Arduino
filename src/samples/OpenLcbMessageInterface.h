/**
 * A work in progress, from an email by
 * dpharris November 2009
 */

typedef struct {
    int status;
    // status of message: 0=failed, 1=empty, 2=waiting; 3=successfully sent
    int priority; // priority of message
    NID_type source; // node identity of source of message
    NID_type destination; // node identity of destination of message
    int type; // format of data
    word nDataBytes; // number of data-bytes
    byte data[]; // the data
} oLCB_buffer_type;

enum {EMPTY, FAILED, WAITING, SENT} buffer_status;

int anoLCBTransportOpen(channel, optional parameters);
/* - open and intialize the transport for a channel
 * - retports 0=failure, !0 for success
 */

int anoLCBTransportReceive(channel, oLCBbuffer);
/* - queries whether anything is received,
 * - returns 0 for none, or >0=number of bytes
 */

int anoLCBTransportSendWait(channel, oLCBbuffer, priority, timeout);
/* - sends a oLCB message
 * - priority sets priority, so might bypass FIFO, or use priority-buffers.
 * - if timeout==0, sends immediately, else waits up to timeout ms
 * - returns 0=failed, !0=success
 */
 
int anoLCBTransportClose(channel);
/* - close the transport.
 */
 
int aTransportControl(channel, parameterName, parameterValue);
/* - implementing unspecified actions, such as
 *   setting priorities, number of buffers, use of FIFO, etc.
 *   for this particular channel
 */
