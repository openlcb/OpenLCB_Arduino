/*
 * $Id: CAN2EtherNetServer.pde 1199 2011-04-03 01:19:01Z jacobsen $ 
 * 
 * CAN2EtherNetServer provides an Ethernet adapter for CAN frames
 * forming the "server" end of a client-server connection. Generally,
 * this is the connection for e.g. PC or iPhone programs.
 * 
 * This is not a full OpenLCB node, but rather a Ethernet-based
 * CAN adapter; it has no OpenLCB-specific processing.
 * 
 * Frames are sent and received in the GridConnect format:
 *   :X182DF285N0203040506080082;
 * where the extended header is between X and N,
 * and the payload follows the N. Messages to Ethernet are
 * followed by "\n". 
 * 
 * For information on the format, see:
 *   http://www.gridconnect.com/canboandto.html
 * 
 * Inbound messages are parsed between
 * the ":" and ";" with characters outside that ignored.
 * Frame validity is not checked.
 *
 * Works with an Ethernet shield based on the WIZnet chip.
 * See below for inclusion is a modified library to avoid
 * a pin conflict with some CAN interfaces.
 * 
 * Up to four Telnet connections are supported automatically
 * by the Ethernet library
 */

#include "Arduino.h"
  
// Addressing information for this node.
// Used for IP address and MAC address
#define ADDR_BYTE_1 10
#define ADDR_BYTE_2  0
#define ADDR_BYTE_3  1
#define ADDR_BYTE_4 98

#include <ctype.h>
#include <can.h>
#include <stdarg.h>
#include <stdio.h>

#define         RXCAN_BUF_COUNT   32
tCAN 		rxCAN[RXCAN_BUF_COUNT]; // CAN receive buffers
int             rxCanBuffCounter;
bool            rxCANflag[RXCAN_BUF_COUNT];
int             rxCanFlagCounter;

tCAN 		txCAN;	// CAN send buffer
tCAN		* ptxCAN;

char    	strBuf[10] ;	// String Buffer
#define 	RX_BUF_SIZE	32

#define         BAUD_RATE       115200
//#define         BAUD_RATE       333333


char    	rxBuff[RX_BUF_SIZE];    // :lddddddddldddddddddddddddd:0
int		rxIndex;
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

// Ethernet requires SPI library since Arduino 19
#if defined(ARDUINO) && ARDUINO > 18
#include <SPI.h>
#endif

// Ethernet2.h for pin 9 CS, Ethernet.h for default pin 10
#include <Ethernet2.h>

// network configuration.  gateway and subnet are optional.
byte mac[] = { 0xDE, 0xAD, ADDR_BYTE_1, ADDR_BYTE_2, ADDR_BYTE_3, ADDR_BYTE_4 };
byte ip[] = { ADDR_BYTE_1, ADDR_BYTE_2, ADDR_BYTE_3, ADDR_BYTE_4 };
byte gateway[] = { ADDR_BYTE_1, ADDR_BYTE_2, ADDR_BYTE_3, 1 };
byte subnet[] = { 255, 255, 255, 0 };

// telnet defaults to port 23
EthernetServer server(23);

void setup()
{
#ifdef ENABLE_DEBUG_MESSAGES
  Serial.begin(BAUD_RATE);
  Serial.println();
  Serial.println(":I LEDuino CAN-EtherNet Server Version: 1;");
#endif

  // Initialize MCP2515
  can_init(BITRATE_125_KBPS);

  rxCanBuffCounter = 0;
  rxCanFlagCounter = 0;

  // Dump out the CAN Controller Registers
  //Serial.println(":I Before regdump ;");
  //can_regdump();
  //Serial.println(":I After regdump ;");
  
  ptxCAN = NULL;

  // Ethernet, from WebClient demo
  Ethernet.begin(mac, ip);

  // start listening for clients
  server.begin();
#ifdef ENABLE_DEBUG_MESSAGES
  Serial.println("Server begun");
#endif

}

uint8_t outBuff[30];
int outBuffIndex;

void storeInOutBuff(char c) {
    outBuff[outBuffIndex++] = c;
}

void loop()
{
  EthernetClient client = server.available();
    
  if(!ptxCAN)
  {  
    if(client == true)
    {
      int rxChar = client.read();
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
          
          // and echo back, in case there's more than one connection
          rxBuff[rxIndex] = '\n';
          //server.write((const uint8_t*)rxBuff, rxIndex+1);
          
        }
        rxIndex = 0;
        break;

      default:	// Everything else must be a character to store
        if( rxIndex < RX_BUF_SIZE )
          rxBuff[rxIndex++] = rxChar & 0x00FF;
        break;
      }
    }
  }
  
  if(ptxCAN && can_check_free_buffer())
  {
    if(can_send_message(ptxCAN)) {
      ptxCAN = NULL; 
    }
  }
  
  while (can_get_message(&rxCAN[rxCanBuffCounter]))  // as many as needed
  {
    // handle message from CAN by marking and moving to next
    rxCANflag[rxCanBuffCounter] = true;
    rxCanBuffCounter++;
    if (rxCanBuffCounter >= RXCAN_BUF_COUNT) rxCanBuffCounter = 0;
  }
  if (rxCANflag[rxCanFlagCounter])
  {
     rxCANflag[rxCanFlagCounter] = false;
     
    outBuffIndex = 0;
    storeInOutBuff(':');
    storeInOutBuff(rxCAN[rxCanFlagCounter].flags.extended ? 'X' : 'S');

    char* substring = strupr(ultoa(rxCAN[rxCanFlagCounter].id, strBuf, 16));
    int i = 0;
    char c;
    while ( (c = substring[i++]) != 0 ) storeInOutBuff(c);

    if(rxCAN[rxCanFlagCounter].flags.rtr)
    {
      storeInOutBuff('R');
      storeInOutBuff('0' + rxCAN[rxCanFlagCounter].length);
    }
    else
    {
      storeInOutBuff('N');
      for( uint8_t i = 0; i < rxCAN[rxCanFlagCounter].length; i++)
      {
        printHexChar(rxCAN[rxCanFlagCounter].data[i]);
      }
    }
    storeInOutBuff(';');
    storeInOutBuff('\n');
    server.write(outBuff, outBuffIndex);
    memset(&rxCAN[rxCanFlagCounter], 0, sizeof(tCAN));
    // increment to next
    rxCanFlagCounter++;
    if (rxCanFlagCounter >= RXCAN_BUF_COUNT) rxCanFlagCounter = 0;
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
  storeInOutBuff(tmp);

  tmp = val & 0x0f;

  if (tmp > 9) 
    tmp += 'A' - 10;
  else 
    tmp += '0';
  storeInOutBuff(tmp);
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
