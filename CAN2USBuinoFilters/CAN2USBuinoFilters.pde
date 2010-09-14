#include <WProgram.h>

#include <ctype.h>
#include <CAN.h>
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
//#define         BAUD_RATE       115200
#define         BAUD_RATE       333333


char    	rxBuff[RX_BUF_SIZE];    // :lddddddddldddddddddddddddd:0
uint8_t		rxIndex;
uint16_t	rxChar;

// -----------------------------------------------------------------------------
void printHexChar(const uint8_t val);
uint8_t hex_to_byte(char *s);
uint8_t char_to_byte(char *s);
tCAN *parseCANStr(char *pBuf, tCAN *pCAN, uint8_t len);

int  printf(const char *__fmt,...)
{
  va_list ap;
  char _str[32]; // 32 chars max!  increase if required to avoid overflow
  
  va_start(ap, __fmt);
  vsnprintf(_str, 32,__fmt, ap);
  Serial.print(_str);
}

/* If you want to receive both 11 and 29 bit identifiers, set your filters
 * and masks as follows:
 */
prog_uint8_t can_filter[] = 
{
	// Group 0
	MCP2515_FILTER(0),				// Filter 0
	MCP2515_FILTER(0),				// Filter 1
	
	// Group 1
	MCP2515_FILTER_EXTENDED(0x00040000),		// Filter 2
	MCP2515_FILTER_EXTENDED(0x00040000),		// Filter 3
	MCP2515_FILTER_EXTENDED(0x00040000),		// Filter 4
	MCP2515_FILTER_EXTENDED(0x00040000),		// Filter 5
	
	MCP2515_FILTER(0),				// Mask 0 (for group 0)
	MCP2515_FILTER_EXTENDED(0),	    	        // Mask 1 (for group 1)
};
// You can receive 11 bit identifiers with either group 0 or 1.
#define SELECT_MCP2515 PORTB &= ~0x04
#define DESELECT_MCP2515 PORTB |= 0x04
//#define SPDR 0x2E
//#define SPSR 0x2D
//#define SPIF 0x80
uint8_t spi_putc(uint8_t abyte) {
    SPDR=abyte;
    while(!(SPSR & (1<<SPIF)) ) {};
    return SPDR;
} 
#define SPI_READ 0x03
uint8_t can_reg_read( uint8_t address)
{
    uint8_t data;
    SELECT_MCP2515;
    spi_putc(SPI_READ);
    spi_putc(address);
    data=spi_putc(0xFF);  // dummy send
    DESELECT_MCP2515;
    return data;
}
#define SPI_BIT_MODIFY 0x05
void can_bit_modify(uint8_t address, uint8_t mask, uint8_t data)
{
	SELECT_MCP2515;
	spi_putc(SPI_BIT_MODIFY);
	spi_putc(address);
	spi_putc(mask);
	spi_putc(data);
	DESELECT_MCP2515;
}
#define CANSTAT 0x0E
#define CANCTRL 0x0F
void can_change_op_mode(uint8_t mode)
{
	can_bit_modify(CANCTRL, 0xe0, mode);
	while ((can_reg_read(CANSTAT) & 0xe0) != (mode & 0xe0)) {};
}
#define CANSTAT 0x0E
#define REQOP2 7
void can_register_dump(void)
{
	uint8_t mode = can_reg_read( CANSTAT );
	// change to configuration mode
	can_change_op_mode( (1<<REQOP2) );
	printf("MCP2515 Regdump:\r\n");
	uint8_t i;
	for (i=0; i < 16; i++) {
		printf("%02x: %02x ", i, can_reg_read(i));
		printf("%02x: %02x ", i+16*1, can_reg_read(i+16*1));
		printf("%02x: %02x ", i+16*2, can_reg_read(i+16*2));
		printf("%02x: %02x ", i+16*3, can_reg_read(i+16*3));
		printf("%02x: %02x ", i+16*4, can_reg_read(i+16*4));
		printf("%02x: %02x ", i+16*5, can_reg_read(i+16*5));
		printf("%02x: %02x ", i+16*6, can_reg_read(i+16*6));
		printf("%02x: %02x\r\n", i+16*7, can_reg_read(i+16*7));
	}
	can_change_op_mode( mode );
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

	// Load filters and masks
	can_static_filter(can_filter);

	
  // Dump out the CAN Controller Registers
  can_register_dump();
}

void loop()
{
  int charWaiting = Serial.available();
  
  if( charWaiting < RX_BUF_LOW )
    digitalWrite(RX_CTS_PIN,LOW);
    
  else if( charWaiting > RX_BUF_HIGH )
    digitalWrite(RX_CTS_PIN,HIGH);
    
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
        if(ptxCAN && can_check_free_buffer())
        {
          if(can_send_message(ptxCAN))
            ptxCAN = NULL; 
        }
      }
      rxIndex = 0;
      break;

    default:			// Everything else must be a 
      if( rxIndex < RX_BUF_SIZE )
        rxBuff[rxIndex++] = rxChar & 0x00FF;
      break;
    }
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
