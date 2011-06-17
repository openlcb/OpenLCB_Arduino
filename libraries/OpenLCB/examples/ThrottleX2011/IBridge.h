#ifndef __IBRIDGE_H__
#define __IBRIDGE_H__

#include <avr/pgmspace.h>
#include <WProgram.h>

//Define the hardware operation function
void IBridge_GPIO_Config(void);

void IBridge_init(void);

unsigned char IBridge_Read_Key(void);

#endif
