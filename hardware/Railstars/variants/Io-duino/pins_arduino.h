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

#ifndef PCICR
#define PCICE EICR
#endif
#ifndef PCMSK
#define PCMSK EIMSK
#endif

#define NUM_DIGITAL_PINS            26
#define NUM_ANALOG_INPUTS           8

#define analogInputToDigitalPin(p)  ((p < 8) ? (p) + 16 : -1)

#define digitalPinHasPWM(p)         ((p) == 2 || (p) == 3 || (p) == 4 || (p) == 5 || (p) == 6 || (p) == 9)

const static uint8_t SS   = 10;
const static uint8_t MOSI = 11;
const static uint8_t MISO = 12;
const static uint8_t SCK  = 13;

const static uint8_t SDA = 24;
const static uint8_t SCL = 25;
const static uint8_t LED_BUILTIN = 14;
const static uint8_t BLUE = 14;
const static uint8_t GOLD = 15;

const static uint8_t A0 = 16;
const static uint8_t A1 = 17;
const static uint8_t A2 = 18;
const static uint8_t A3 = 19;
const static uint8_t A4 = 20;
const static uint8_t A5 = 21;
const static uint8_t A6 = 22;
const static uint8_t A7 = 23;

//PC interrupts are not available on the AT90CAN128
#define digitalPinToPCICR(p)    (0)
#define digitalPinToPCICRbit(p) (1)
#define digitalPinToPCMSK(p)    ((uint8_t *)0)
#define digitalPinToPCMSKbit(p) (p)

#ifdef ARDUINO_MAIN

// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
#define PA 1
#define PB 2
#define PC 3
#define PD 4
#define PE 5
#define PF 6
#define PG 7


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
	PE, /* 0 */
	PE, /* 1 */
	PE, /* 2 */
	PE, /* 3 */
	PE, /* 4 */
	PB, /* 5 */
	PB, /* 6 */
	PE, /* 7 */
	PE, /* 8 */
	PB, /* 9 */
	PB, /* 10 */
	PB, /* 11 */
	PB, /* 12 */
	PB, /* 13 */
	PG, /* 14 (blue) */
	PG, /* 15 (gold) */
	PF, /* 16 (A0) */
	PF, /* 17 (A1) */
	PF, /* 18 (A2) */
	PF, /* 19 (A3) */
	PF, /* 20 (A4) */
	PF, /* 21 (A5) */
	PF, /* 22 (A6) */
	PF, /* 23 (A7) */
	PD, /* 24 (SDA) */
	PD, /* 25 (SCL) */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	_BV(0), /* 0, port E */
	_BV(1), /* 1, port E */
	_BV(3), /* 2, port E */
	_BV(4), /* 3, port E */
	_BV(5), /* 4, port E */
	_BV(4), /* 5, port B */
	_BV(5), /* 6, port B */
	_BV(6), /* 7, port E */
	_BV(2), /* 8, port E */
	_BV(6), /* 9, port B */
	_BV(0), /* 10, port B */
	_BV(2), /* 11, port B */
	_BV(3), /* 12, port B */
	_BV(1), /* 13, port B */
	_BV(0), /* 14, port G (blue) */
	_BV(1), /* 15, port G (gold) */
	_BV(0), /* 16, port F (A0) */
	_BV(1), /* 17, port F (A1) */
	_BV(2), /* 18, port F (A2) */
	_BV(3), /* 19, port F (A3) */
	_BV(4), /* 20, port F (A4) */
	_BV(5), /* 21, port F (A5) */
	_BV(6), /* 22, port F (A6) */
	_BV(7), /* 23, port F (A7) */
	_BV(1), /* 24, port D */
	_BV(0), /* 25, port D */
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	NOT_ON_TIMER, /* 0 */
	NOT_ON_TIMER, /* 1 */
	TIMER3A,	  /* 2 */
	TIMER3B,      /* 3 */
	TIMER3C, 	  /* 4 */
	TIMER2A,      /* 5 */
	TIMER1A,      /* 6 */
	NOT_ON_TIMER, /* 7 */
	NOT_ON_TIMER, /* 8 */
	TIMER1B,      /* 9 */
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
};

#endif

#endif
