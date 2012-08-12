#include "FT245.h"

FTDI_FT245 FT245(&DDRF, &PORTF, &PINF, &DDRD, &PORTD, &PIND, 3, 2, 1, 0);

void setup()
{
#if defined(JTD)
  uint8_t DISABLE_JTAG = MCUCR | (1<<JTD);
  MCUCR = DISABLE_JTAG;
  MCUCR = DISABLE_JTAG;
#endif

  Serial.begin(115200);
  Serial.println("FTDI FT245 Test");

  FT245.begin();
};

void loop()
{
  if(FT245.available())
    Serial.write(FT245.read());
    
  if(Serial.available())
    FT245.write(Serial.read());
};
