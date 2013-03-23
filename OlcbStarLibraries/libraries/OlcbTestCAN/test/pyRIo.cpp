#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

/**
 * Scaffold for testing a CAN-based Arduino sketch
 * via python
 */
 
// Sketch to test 
#include <../../../OlcbBasicNode/OlcbBasicNode.ino>


// infrastructure from here down
uint8_t char_to_byte(char *s)
{
  uint8_t t = *s;

  if (t >= 'a')
    t = t - 'a' + 10;
  else if (t >= 'A')
    t = t - 'A' + 10;
  else
    t = t - '0';

  return t;
}

uint8_t hex_to_byte(char *s)
{
  return (char_to_byte(s) << 4) | char_to_byte(s + 1);
}

// Parser from CAN2USBino

void parseCANStr(char *pBuf, OpenLcbCanBuffer *pCAN, uint8_t len)
{
  if( (pBuf[0] == ':') && (pBuf[len-1] == ';') && (len >= 4) && ( (pBuf[1] == 'X') || (pBuf[1] == 'S') ) )
  {
    memset(pCAN, 0, sizeof(tCAN));

    pCAN->flags.extended = pBuf[1] == 'X';

    char *pEnd;

    pCAN->id = strtoul(pBuf+2, &pEnd, 16);
    
    // If Standard Frame then check to see if Id is in the valid 11-bit range
    if((pCAN->flags.extended && (pCAN->id > 0x1FFFFFFF)) || (!pCAN->flags.extended && (pCAN->id > 0x07FF)))
      return;

    if(*pEnd == 'N')
    {
      pEnd++;
      pCAN->length = 0;
      while(isxdigit(*pEnd))
      {
        pCAN->data[pCAN->length] = hex_to_byte(pEnd);
        pCAN->length++;
        pEnd += 2;
      }
    }
    else if(*pEnd == 'R')
    {  
      pCAN->flags.rtr = 1;
      char tChar = *(pEnd+1);
      if(isdigit(tChar))
        pCAN->length = *(pEnd+1) - '0';
    }
    return;
  }
  return;
} 


tCAN 		txCAN;

void OpenLcb_can_init() {
}

bool sentOutput; 

void printBuffer(OpenLcbCanBuffer* b) {
    printf(":X%08XN", b->id);
    for (int i = 0 ; i<b->length; i++) {
        printf("%02X", b->data[i]);
    }
    printf(";\n");
    fflush(stdout);
    sentOutput = true;
    
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
    printBuffer(b);
    return true;
}

// Queue a CAN frame for sending; spins until it can queue
void OpenLcb_can_queue_xmt_wait(OpenLcbCanBuffer* b) {
    printBuffer(b);
}

// Send a CAN frame, waiting until it has been sent
void OpenLcb_can_send_xmt(OpenLcbCanBuffer* b) {
    printBuffer(b);
}

// Check whether all frames have been sent,
// a proxy for the link having gone idle
bool OpenLcb_can_xmt_idle() {
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
        return  true;
    }
}

void queueTestMessage(OpenLcbCanBuffer* b) {
    queuedBuffer = b;
}

OpenLcbCanBuffer b;

/**
 * getline is a GNU extension, provide, see
 * http://stackoverflow.com/questions/735126/are-there-alternate-implementations-of-gnu-getline-interface
 */
ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (n == NULL) {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = (char *) malloc(128);
        if (bufptr == NULL) {
                return -1;
        }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((p - bufptr) > (size - 1)) {
                size = size + 128;
                bufptr = (char *) realloc(bufptr, size);
                if (bufptr == NULL) {
                        return -1;
                }
        }
        *p++ = c;
        if (c == '\n') {
                break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}

int main()
{
        
    // get start buffer
    size_t len = 80 * sizeof (char);
    char* ptr = (char*) malloc(len);
    
    // call sketch's startup
    setup();
    
    // call loop at few times
    while (1) {
        // run until boring
        do {
            sentOutput = false;
            for (int i = 0; i < 2; i++) loop();
        } while (sentOutput);
        
        // say done
        printf("\n");
        fflush(stdout);
        
        // wait for input (don't know how to check for input available)
        ptr[0] = 0;
        getline(&ptr, &len, stdin);
        if (ptr[0] == ':') {
            // process frame
            parseCANStr(ptr, &b, index(ptr, ';')-ptr+1);
            queueTestMessage(&b);
        } else if (ptr[0] == 'T') {
            tick(50);
        }
    }
    
    exit(0);
    
}

/* -----------------------------------------------------------------------------
 Parser Test Cases
:X1234N12345678;
:XN;	(not valid but we parse to :X0N;
:SN;	(not valid but we parse to :S0N;
:SR0;	(not valid but we parse to :S0R0;
:SR;	(not valid but we parse to :S0R0; 
:S1R8;
:S1N11;
:S3FFN;
:S7FFN;
:S8FFN;	(Invalid)
:X1FFFFFFFN1234567812345678;
:X2FFFFFFFN1234567812345678; (Invalid)
:S123N01;:S123N0002;:S123N000003;:S123N00000004;:S123N0000000005;:S123N000000000006;:X22N11;:X22N12;:X22N13;:X22N14;:X22N15;:X212N16;:X23312N17;:S123N18;
-----------------------------------------------------------------------------*/
