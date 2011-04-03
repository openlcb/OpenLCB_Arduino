/*
 * $Id$ 
 */
#include <WProgram.h>

#include <ctype.h>
#include <can.h>
#include <stdarg.h>
#include <stdio.h>

tCAN 		rxCAN;	// CAN receive buffer
tCAN 		txCAN;	// CAN send buffer
tCAN		* ptxCAN;

char    	strBuf[10] ;	// String Buffer

#define 	RX_BUF_SIZE	32
#define         RX_CTS_PIN      9
#define         RX_BUF_LOW      32 
#define         RX_BUF_HIGH     96
#define         BAUD_RATE       115200
//#define         BAUD_RATE       333333


char    	rxBuff[RX_BUF_SIZE];    // :lddddddddldddddddddddddddd:0
uint8_t		rxIndex;
uint16_t	rxChar;

// -----------------------------------------------------------------------------
void printHexChar(const uint8_t val);
uint8_t hex_to_byte(char *s);
uint8_t char_to_byte(char *s);
tCAN *parseCANStr(char *pBuf, tCAN *pCAN, uint8_t len);


#define ENABLE_DEBUG_MESSAGES
extern "C" {
void  debugf(const char *__fmt,...)
{
#ifdef ENABLE_DEBUG_MESSAGES
  va_list ap;
  char _str[32]; // 32 chars max!  increase if required to avoid overflow
  
  va_start(ap, __fmt);
  vsnprintf(_str, 32,__fmt, ap);
  Serial.print(_str);
#endif
}
}


void setup()
{
  pinMode(RX_CTS_PIN,OUTPUT);
  digitalWrite(RX_CTS_PIN,LOW);

  Serial.begin(BAUD_RATE);
  Serial.println();
  Serial.println(":I LEDuino CAN-USB Adaptor Version: 1;");

  // Initialize MCP2515
  can_init(BITRATE_125_KBPS);
	
  // Dump out the CAN Controller Registers
  //Serial.println(":I Before regdump ;");
  //can_regdump();
  //Serial.println(":I After regdump ;");
  
  ptxCAN = NULL;
}

bool isTxBufferFree(void)
{
  uint8_t status = can_buffers_status();

    // Check to see if Tx Buffer 0 or 1 is free	
  if ((status & (ST_TX0REQ|ST_TX1REQ)) == (ST_TX0REQ|ST_TX1REQ))
    return false;  //  Both at full
  else
    return true;   // At least 1 buffer was free
}


void loop()
{
  int charWaiting = Serial.available();
  
  if( charWaiting < RX_BUF_LOW )
    digitalWrite(RX_CTS_PIN,LOW);
    
  else if( charWaiting > RX_BUF_HIGH )
    digitalWrite(RX_CTS_PIN,HIGH);
  
  if(!ptxCAN)
  {  
    int rxChar = Serial.read();
    if(rxChar >= 0)
    {
      switch(rxChar)
      {
      case ':':
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

      default:			// Everything else must be a 
        if( rxIndex < RX_BUF_SIZE )
          rxBuff[rxIndex++] = rxChar & 0x00FF;
        break;
      }
    }
  }

  if(ptxCAN && isTxBufferFree())
  {
    if(can_send_message(ptxCAN))
      ptxCAN = NULL; 
  }
  
  if(can_get_message(&rxCAN))
  {
    Serial.print(':');
    Serial.print(rxCAN.flags.extended ? 'X' : 'S');

    Serial.print(strupr(ultoa(rxCAN.id, strBuf, 16)));

    if(rxCAN.flags.rtr)
    {
      Serial.print('R');
      Serial.print('0' + rxCAN.length);
    }
    else
    {
      Serial.print('N');
      for( uint8_t i = 0; i < rxCAN.length; i++)
      {
        printHexChar(rxCAN.data[i]);
      }
    }
    Serial.println(';');

    memset(&rxCAN, 0, sizeof(tCAN));
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

tCAN *parseCANStr(char *pBuf, tCAN *pCAN, uint8_t len)
{
  if( (pBuf[0] == ':') && (pBuf[len-1] == ';') && (len >= 4) && ( (pBuf[1] == 'X') || (pBuf[1] == 'S') ) )
  {
    memset(pCAN, 0, sizeof(tCAN));

    pCAN->flags.extended = pBuf[1] == 'X';

    char *pEnd;

    pCAN->id = strtoul(pBuf+2, &pEnd, 16);

    // If Standard Frame then check to see if Id is in the valid 11-bit range
    if((pCAN->flags.extended && (pCAN->id > 0x1FFFFFFF)) || (!pCAN->flags.extended && (pCAN->id > 0x07FF)))
      return NULL;

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
  Serial.print(tmp);

  tmp = val & 0x0f;

  if (tmp > 9) 
    tmp += 'A' - 10;
  else 
    tmp += '0';
  Serial.print(tmp);
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
