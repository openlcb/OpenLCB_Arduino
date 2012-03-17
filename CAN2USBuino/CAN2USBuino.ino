
/*
 * $Id$ 
 *
 * This sketch allows an Arduino with USB and CAN interfaces to
 * be used as an adapter between them.  The frames are sent in
 * what's commonly called the "GridConnect" protocol.
 *
 * Note that a serial speed of 333333 or above is needed to 
 * be certain of keeping up with a full CAN bus.  Not all
 * computers will connect at that speed.  In particular, 
 * Mac computers with standard FTDI drivers might only have
 * 230,400 or even 115,200 available as a standard rate.
 * By setting 8E2 or even better 8N2, 230400 here can be used with
 * a Mac at 230400 despite some internal baud issue in the Arduino baud generation.
 *
 * For data directed from USB -> CAN, the sketch provides flow
 * control using the (virtual) CTS signal.
 */

#define         BAUD_RATE       230400
//#define         BAUD_RATE       333333

#include <arduino.h>

#include <ctype.h>
#include <can.h>
#include <stdarg.h>
#include <stdio.h>

#define         RXCAN_BUF_COUNT   32
tCAN 		rxCAN[RXCAN_BUF_COUNT]; // CAN receive buffers
int             rxCanBuffCounter;
bool            rxCANflag[RXCAN_BUF_COUNT]; // true is contains data
int             rxCanFlagCounter;

tCAN 		txCAN;	// CAN send buffer
tCAN		* ptxCAN;


#define         RX_WAIT_LOW      4 
#define         RX_WAIT_HIGH     16

#define 	RX_BUF_SIZE	64
char    	rxBuff[RX_BUF_SIZE];    // :lddddddddldddddddddddddddd:0 times 2 for doubled protocol
uint8_t		rxIndex;
uint16_t	rxChar;

// -----------------------------------------------------------------------------
void printHexChar(const uint8_t val);
uint8_t hex_to_byte(char *s);
uint8_t char_to_byte(char *s);
tCAN *parseCANStr(char *pBuf, tCAN *pCAN, uint8_t len);


void setup()
{
  Serial.begin(BAUD_RATE);
  Serial.println();
  Serial.println(":I LEDuino CAN-USB Adaptor Version 2;");

  // Initialize MCP2515
  can_init(BITRATE_125_KBPS);
  rxCanBuffCounter = 0;
  rxCanFlagCounter = 0;
	
  ptxCAN = NULL;
}

void saveCanFrames() {
  // capture as many input frames as possible
  while (can_get_message(&rxCAN[rxCanBuffCounter])) 
  {
    // handle message from CAN by marking and moving to next
    rxCANflag[rxCanBuffCounter] = true;
    rxCanBuffCounter++;
    if (rxCanBuffCounter >= RXCAN_BUF_COUNT) rxCanBuffCounter = 0;
  }
}

boolean setFlowStop = false;

void loop()
{
  saveCanFrames();
  // check for RTS flow control to PC on USB side
  int charWaiting = Serial.available();
  
  if( setFlowStop && charWaiting < RX_WAIT_LOW ) {
    Serial.print((char)0x11);  // XON
    setFlowStop = false;
  }
  else if( !setFlowStop && charWaiting > RX_WAIT_HIGH ) {
    Serial.print((char)0x13);  // XOFF
    setFlowStop = true;
  }
  
   
  saveCanFrames();
 
  // send one frame from CAN on serial link if possible
  // note that print calls are blocking
  if (rxCANflag[rxCanFlagCounter])
  {
     rxCANflag[rxCanFlagCounter] = false;
    
    Serial.print(':');
    Serial.print(rxCAN[rxCanFlagCounter].flags.extended ? 'X' : 'S');
    saveCanFrames();

    //Serial.print(rxCAN[rxCanFlagCounter].id, HEX);
    printHexChar( (rxCAN[rxCanFlagCounter].id>>24)&0xFF);
    printHexChar( (rxCAN[rxCanFlagCounter].id>>16)&0xFF);
    printHexChar( (rxCAN[rxCanFlagCounter].id>>8)&0xFF);  
    printHexChar( (rxCAN[rxCanFlagCounter].id)&0xFF); 
    
    if(rxCAN[rxCanFlagCounter].flags.rtr)
    {
      Serial.print('R');
      Serial.print('0' + rxCAN[rxCanFlagCounter].length);
    }
    else
    {
      Serial.print('N');
      saveCanFrames();
      for( uint8_t i = 0; i < rxCAN[rxCanFlagCounter].length; i++)
      {
        printHexChar(rxCAN[rxCanFlagCounter].data[i]);
      }
    }
    Serial.println(';');

    memset(&rxCAN[rxCanFlagCounter], 0, sizeof(tCAN));
    // increment to next
    rxCanFlagCounter++;
    if (rxCanFlagCounter >= RXCAN_BUF_COUNT) rxCanFlagCounter = 0;
  } else  if(!ptxCAN) { // character processing slow, only do if don't have anything to send
   
    // transmit buffer free, so we can load it if characters are available from USB
    // handle characters from USB to CAN
    int rxChar = Serial.read();
    if(rxChar >= 0)
    {
      switch(rxChar)
      {
      case ':':
        rxIndex = 0;
        rxBuff[rxIndex++] = rxChar & 0x00FF;
        break;
      case '!':
        rxIndex = 0;
        rxBuff[rxIndex++] = rxChar & 0x00FF;
        break;

      case ';':
        if( rxIndex < RX_BUF_SIZE )
        {
          rxBuff[rxIndex++] = rxChar & 0x00FF;
          rxBuff[rxIndex] = '\0';	// Null Terminate the string

          ptxCAN = parseCANStr(rxBuff, &txCAN, rxIndex);
        }
        rxIndex = 0;
        break;

      case '\n':
      case '\r':
        // ran off end of line, go back to start of buffer
        rxIndex = 0;
        break;
        
      default:			// Everything else must be a byte to send
        if( rxIndex < RX_BUF_SIZE )
          rxBuff[rxIndex++] = rxChar & 0x00FF;
        break;
      }
    }
  }

  // send buffer to CAN asap once full
  if(ptxCAN && can_check_free_buffer())
  {
    if(can_send_message(ptxCAN))
      ptxCAN = NULL; 
  }

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
boolean valid(char c) {

   if (c>='0' && c <= '9') return true;
   if (c>='A' && c <= 'Z') return true;
   if (c>='a' && c <= 'f') return true; // just inc ase lower hex

   return false;
}

tCAN *parseCANStr(char *pBuf, tCAN *pCAN, uint8_t len) {
    if( len >= 4 )  {  // first, last char already known to be right
        memset(pCAN, 0, sizeof(tCAN));

        if (pBuf[0] == '!') {
            // this is a doubled buffer, convert
            int to = 1;
            int from = 1;
            while (from < len) {
               pBuf[to] = pBuf[from];
               if (!valid(pBuf[from])) pBuf[to] = pBuf[from+1];
               to++;
               if (pBuf[from] == pBuf[from+1] || !valid(pBuf[from]) || !valid(pBuf[from+1]) ) from++;  // doubled char; if not, char has been lost
               from++;
            }
            len = to;
        }
        
        pCAN->flags.extended = pBuf[1] == 'X';

        char *pEnd;

        pCAN->id = strtoul(pBuf+2, &pEnd, 16);

        // If Standard Frame then check to see if Id is in the valid 11-bit range
        if((pCAN->flags.extended && (pCAN->id > 0x1FFFFFFF)) || (!pCAN->flags.extended && (pCAN->id > 0x07FF)))
            return NULL;

        if(*pEnd == 'N') {
            pEnd++;
            pCAN->length = 0;
            while(isxdigit(*pEnd)) {
                pCAN->data[pCAN->length] = hex_to_byte(pEnd);
                pCAN->length++;
                pEnd += 2;
            }
        } else if(*pEnd == 'R') {  
            pCAN->flags.rtr = 1;
            char tChar = *(pEnd+1);
            if(isdigit(tChar))
                pCAN->length = *(pEnd+1) - '0';
        }
        return pCAN;
    }
    return NULL;
} 


// -----------------------------------------------------------------------------
void printHexChar(const uint8_t val)
{
  uint8_t tmp = val >> 4;

  if (tmp > 9)
    tmp += 'A' - 10;
  else 
    tmp += '0';
  Serial.print((char)tmp);

  tmp = val & 0x0f;

  if (tmp > 9) 
    tmp += 'A' - 10;
  else 
    tmp += '0';
  Serial.print((char)tmp);
}
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
uint8_t hex_to_byte(char *s)
{
  return (char_to_byte(s) << 4) | char_to_byte(s + 1);
}
