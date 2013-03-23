/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.h 249 2007-02-03 16:52:51Z mellis $
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

#ifndef __RAILSTARS_IO__
#define __RAILSTARS_IO__
#endif

#ifndef PCICR
#define PCICE EICR
#endif
#ifndef PCMSK
#define PCMSK EIMSK
#endif

#define NUM_DIGITAL_PINS            50
#define NUM_ANALOG_INPUTS           8

#define analogInputToDigitalPin(p)  ((p < 8) ? (p) + 40 : -1)

/* not gonna bother with PWM */
#define digitalPinHasPWM(p)         (0)

const static uint8_t SS   = 16;
const static uint8_t MOSI = 18;
const static uint8_t MISO = 19;
const static uint8_t SCK  = 17;

const static uint8_t SDA = 25;
const static uint8_t SCL = 24;
const static uint8_t LED_BUILTIN = 48;
#define BLUE 48
#define GOLD 49

const static uint8_t A0 = 40;
const static uint8_t A1 = 41;
const static uint8_t A2 = 42;
const static uint8_t A3 = 43;
const static uint8_t A4 = 44;
const static uint8_t A5 = 45;
const static uint8_t A6 = 46;
const static uint8_t A7 = 47;

//PC interrupts are not available on the AT90CAN128
#define digitalPinToPCICR(p)    (0)
#define digitalPinToPCICRbit(p) (1)
#define digitalPinToPCMSK(p)    ((uint8_t *)0)
#define digitalPinToPCMSKbit(p) (p)

#ifdef ARDUINO_MAIN

// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT,
	(uint16_t) &DDRA,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
	(uint16_t) &DDRE,
	(uint16_t) &DDRF,
	(uint16_t) &DDRG,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	(uint16_t) &PORTA,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
	(uint16_t) &PORTE,
	(uint16_t) &PORTF,
	(uint16_t) &PORTG,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PORT,
	(uint16_t) &PINA,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
	(uint16_t) &PINE,
	(uint16_t) &PINF,
	(uint16_t) &PING,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
	PA, /* 0 OUTPUTS */
	PA, /* 1 */
	PA, /* 2 */
	PA, /* 3 */
	PA, /* 4 */
	PA, /* 5 */
	PA, /* 6 */
	PA, /* 7 */
	PC, /* 8 INPUTS */
	PC, /* 9 */
	PC, /* 10 */
	PC, /* 11 */
	PC, /* 12 */
	PC, /* 13 */
	PC, /* 14 */
	PC, /* 15 */
	PB, /* 16 PORT B*/
	PB, /* 17 */
	PB, /* 18 */
	PB, /* 19 */
	PB, /* 20 */
	PB, /* 21 */
	PB, /* 22 */
	PB, /* 23 */
	PD, /* 24 PORT D/G */
	PD, /* 25 */
	PD, /* 26 */
	PD, /* 27 */
	PD, /* 28 */
	PG, /* 29 PD5 is CAN: Sub in PG3 here */
	PG, /* 30 PD6 is CAN: Sub in PG4 here */
	PD, /* 31 */
	PE, /* 32 PORT E */
	PE, /* 33 */
	PE, /* 34 */
	PE, /* 35 */
	PE, /* 36 */
	PE, /* 37 */
	PE, /* 38 */
	PE, /* 39 */
	PF, /* 40 PORT F */
	PF, /* 41 */
	PF, /* 42 */
	PF, /* 43 */
	PF, /* 44 */
	PF, /* 45 */
	PF, /* 46 */
	PF, /* 47 */
	PG, /* 48 BLUE */
	PG, /* 49 GOLD */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	_BV(0), /* 0, port A */
	_BV(1), /* 1, port A */
	_BV(2), /* 2, port A */
	_BV(3), /* 3, port A */
	_BV(4), /* 4, port A */
	_BV(5), /* 5, port A */
	_BV(6), /* 6, port A */
	_BV(7), /* 7, port A */
	
	_BV(0), /* 0, port C */
	_BV(1), /* 1, port C */
	_BV(2), /* 2, port C */
	_BV(3), /* 3, port C */
	_BV(4), /* 4, port C */
	_BV(5), /* 5, port C */
	_BV(6), /* 6, port C */
	_BV(7), /* 7, port C */
	
	_BV(0), /* 0, port B */
	_BV(1), /* 1, port B */
	_BV(2), /* 2, port B */
	_BV(3), /* 3, port B */
	_BV(4), /* 4, port B */
	_BV(5), /* 5, port B */
	_BV(6), /* 6, port B */
	_BV(7), /* 7, port B */
	
	_BV(0), /* 0, port D */
	_BV(1), /* 1, port D */
	_BV(2), /* 2, port D */
	_BV(3), /* 3, port D */
	_BV(4), /* 4, port D */
	_BV(3), /* 3, port G */
	_BV(4), /* 4, port G */
	_BV(7), /* 7, port D */
	
	_BV(0), /* 0, port E */
	_BV(1), /* 1, port E */
	_BV(2), /* 2, port E */
	_BV(3), /* 3, port E */
	_BV(4), /* 4, port E */
	_BV(5), /* 5, port E */
	_BV(6), /* 6, port E */
	_BV(7), /* 7, port E */

	_BV(0), /* 0, port F */
	_BV(1), /* 1, port F */
	_BV(2), /* 2, port F */
	_BV(3), /* 3, port F */
	_BV(4), /* 4, port F */
	_BV(5), /* 5, port F */
	_BV(6), /* 6, port F */
	_BV(7), /* 7, port F */
	
	_BV(0), /* 0, port G BLUE */
	_BV(1), /* 1, port G GOLD */
};


/* Just not gonna mess with it! */
const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	NOT_ON_TIMER, /* 0  */
	NOT_ON_TIMER, /* 1  */
	NOT_ON_TIMER, /* 2  */
	NOT_ON_TIMER, /* 3  */
	NOT_ON_TIMER, /* 4  */
	NOT_ON_TIMER, /* 5  */
	NOT_ON_TIMER, /* 6  */
	NOT_ON_TIMER, /* 7  */
	NOT_ON_TIMER, /* 8  */
	NOT_ON_TIMER, /* 9  */
	NOT_ON_TIMER, /* 10 */
	NOT_ON_TIMER, /* 11 */
	NOT_ON_TIMER, /* 12 */
	NOT_ON_TIMER, /* 13 */
	NOT_ON_TIMER, /* 14 */
	NOT_ON_TIMER, /* 15 */
	NOT_ON_TIMER, /* 16 */
	NOT_ON_TIMER, /* 17 */
	NOT_ON_TIMER, /* 18 */
	NOT_ON_TIMER, /* 19 */
	NOT_ON_TIMER, /* 20 */
	NOT_ON_TIMER, /* 21 */
	NOT_ON_TIMER, /* 22 */
	NOT_ON_TIMER, /* 23 */
	NOT_ON_TIMER, /* 24 */
	NOT_ON_TIMER, /* 25 */
	NOT_ON_TIMER, /* 26 */
	NOT_ON_TIMER, /* 27 */
	NOT_ON_TIMER, /* 28 */
	NOT_ON_TIMER, /* 29 */
	NOT_ON_TIMER, /* 30 */
	NOT_ON_TIMER, /* 31 */
	NOT_ON_TIMER, /* 32 */
	NOT_ON_TIMER, /* 33 */
	NOT_ON_TIMER, /* 34 */
	NOT_ON_TIMER, /* 35 */
	NOT_ON_TIMER, /* 36 */
	NOT_ON_TIMER, /* 37 */
	NOT_ON_TIMER, /* 38 */
	NOT_ON_TIMER, /* 39 */
	NOT_ON_TIMER, /* 40 */
	NOT_ON_TIMER, /* 41 */
	NOT_ON_TIMER, /* 42 */
	NOT_ON_TIMER, /* 43 */
	NOT_ON_TIMER, /* 44 */
	NOT_ON_TIMER, /* 45 */
	NOT_ON_TIMER, /* 46 */
	NOT_ON_TIMER, /* 47 */
	NOT_ON_TIMER, /* 48 */
	NOT_ON_TIMER, /* 49 */
};

#endif

#endif
