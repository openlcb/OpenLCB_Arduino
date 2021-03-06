/*
 * $Id: CAN2USBuino.ino 2548 2012-07-28 12:01:29Z kiwi64ajs $ 
 *
 * This sketch allows an Arduino with USB and CAN interfaces to
 * be used as an adapter between them.  The frames are sent in
 * what's commonly called the "GridConnect" protocol.
 *
 * This is for use with boards containing a FT245 USB adapter.
 */

#include <arduino.h>
#include <MsTimer2.h>
#include <FT245.h>

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

#define 	RX_BUF_SIZE	64
char    	rxBuff[RX_BUF_SIZE];    // :lddddddddldddddddddddddddd:0 times 2 for doubled protocol
uint8_t		rxIndex;
uint16_t	rxChar;

#ifndef BLUE
#define BLUE 13 //TX LED indicator
#endif

#ifndef GOLD
#define GOLD 14 //RX LED indicator
#endif

#ifdef BLUE
#define CAN_LED_FLASH_TICKS  10

volatile uint8_t txCANLEDTicks;
volatile uint8_t rxCANLEDTicks;

void flashCANTrafficeLEDs()
{
    // If Tx Ticks > 0, decrement and if 0 set LED pin HIGH
  if( txCANLEDTicks > 0)
  {
    if(--txCANLEDTicks)
      digitalWrite(BLUE, HIGH);
  }

    // If Rx Ticks > 0, decrement and if 0 set LED pin HIGH
  if( rxCANLEDTicks > 0)
  {
    if(--rxCANLEDTicks)
      digitalWrite(GOLD, HIGH);
  }
}
#endif

// -----------------------------------------------------------------------------
void printHexChar(const uint8_t val);
uint8_t hex_to_byte(char *s);
uint8_t char_to_byte(char *s);
tCAN *parseCANStr(char *pBuf, tCAN *pCAN, uint8_t len);

#define AJS
#ifdef AJS
FTDI_FT245 FT245(&DDRF, &PORTF, &PINF, &DDRD, &PORTD, &PIND, 3, 2, 1, 0);
#else
FTDI_FT245 FT245(&DDRC, &PORTC, &PINC, &DDRA, &PORTA, &PINA, 0, 1, 3, 2);
#endif

void setup()
{
  // Disable the JTAG Port
#if defined(JTD)
  uint8_t DISABLE_JTAG = MCUCR | (1<<JTD);
  MCUCR = DISABLE_JTAG;
  MCUCR = DISABLE_JTAG;
#endif

//TX LED Indication
#ifdef BLUE
  pinMode(BLUE, OUTPUT);
  digitalWrite(BLUE, HIGH);
#endif

//RX LED Indication
#ifdef GOLD
  pinMode(GOLD, OUTPUT);
  digitalWrite(GOLD, HIGH);
  
    // Setup Timer Tick of 10ms
  MsTimer2::set(10, flashCANTrafficeLEDs); // 500ms period
  MsTimer2::start();
#endif

  FT245.begin();
  FT245.println();
  FT245.println(":I FTDI FT245 CAN-USB Adaptor Version 1;");
  


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
 
  // send one frame from CAN on serial link if possible
  // note that print calls are blocking
  if (rxCANflag[rxCanFlagCounter])
  {
    rxCANflag[rxCanFlagCounter] = false;

#ifdef GOLD
      //Turn RX LED Indication On
    rxCANLEDTicks = CAN_LED_FLASH_TICKS;
    digitalWrite(GOLD, LOW);
#endif

    FT245.print(':');
    FT245.print(rxCAN[rxCanFlagCounter].flags.extended ? 'X' : 'S');
    saveCanFrames();

    printHexChar( (rxCAN[rxCanFlagCounter].id>>24)&0xFF);
    printHexChar( (rxCAN[rxCanFlagCounter].id>>16)&0xFF);
    printHexChar( (rxCAN[rxCanFlagCounter].id>>8)&0xFF);  
    printHexChar( (rxCAN[rxCanFlagCounter].id)&0xFF); 
    
    if(rxCAN[rxCanFlagCounter].flags.rtr)
    {
      FT245.print('R');
      FT245.print('0' + rxCAN[rxCanFlagCounter].length);

    }
    else
    {
      FT245.print('N');
      saveCanFrames();
      for( uint8_t i = 0; i < rxCAN[rxCanFlagCounter].length; i++)
      {
        printHexChar(rxCAN[rxCanFlagCounter].data[i]);
      }
    }
    FT245.println(';');

    memset(&rxCAN[rxCanFlagCounter], 0, sizeof(tCAN));
    // increment to next
    rxCanFlagCounter++;
    if (rxCanFlagCounter >= RXCAN_BUF_COUNT) rxCanFlagCounter = 0;
  } else  if(!ptxCAN) { // character processing slow, only do if don't have anything to send

    // transmit buffer free, so we can load it if characters are available from USB
    // handle characters from USB to CAN
    int rxChar = FT245.read();
    
    if(rxChar >= 0)
    {
#ifdef BLUE
        //Turn Tx Indication LED ON    
      txCANLEDTicks = CAN_LED_FLASH_TICKS;
      digitalWrite(BLUE, LOW);
#endif 
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

        // Ignore any XON/XOFF Chars
      case 0x11:
      case 0x13:
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
  FT245.print((char)tmp);

  tmp = val & 0x0f;

  if (tmp > 9) 
    tmp += 'A' - 10;
  else 
    tmp += '0';
  FT245.print((char)tmp);
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
