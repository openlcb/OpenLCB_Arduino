/****************************************************************************
 *  FT245.cpp - FTDI library
 *  
 * 	Copyright (C) 2012 Alex Shepherd
 * 
 * 	This library is free software; you can redistribute it and/or
 * 	modify it under the terms of the GNU Lesser General Public
 * 	License as published by the Free Software Foundation; either
 * 	version 2.1 of the License, or (at your option) any later version.
 * 
 * 	This library is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * 	Lesser General Public License for more details.
 * 
 * 	You should have received a copy of the GNU Lesser General Public
 * 	License along with this library; if not, write to the Free Software
 * 	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Inspired by the code found here: 
	// Web Link		: http://www.codeforge.com/read/156091/ft245_io.c__html
	// Title		: FTDI FT245 USB Inferface Driver Library
	// Author		: Pascal Stang - Copyright (C) 2004
	// Created		: 2004.02.10
 */

#include "wiring_private.h"
#include "FT245.h"

// Constructors ////////////////////////////////////////////////////////////////

FTDI_FT245::FTDI_FT245(
  volatile uint8_t *data_ddr, volatile uint8_t *data_port, volatile uint8_t *data_pin,
  volatile uint8_t *cont_ddr, volatile uint8_t *cont_port, volatile uint8_t *cont_pin,
  uint8_t rd, uint8_t wr, uint8_t rxf, uint8_t txe )
{
  _data_ddr = data_ddr;
  _data_port = data_port;
  _data_pin = data_pin;
  _cont_ddr = cont_ddr;
  _cont_port = cont_port;
  _cont_pin = cont_pin;
  _rd = rd;
  _wr = wr;
  _rxf = rxf;
  _txe = txe;
  _peekBuf = -1;
}

// Public Methods //////////////////////////////////////////////////////////////

void FTDI_FT245::begin()
{
  *_data_ddr = 0x00;    // Put Port DDR Back to Input
  *_data_port = 0xFF;   // Enable The Pull-Ups

  // Set the Control Pins to Output  
  sbi(*_cont_ddr, _rd);
  sbi(*_cont_ddr, _wr);
  
  // Set the control line initial states
  sbi(*_cont_port, _rd);
  cbi(*_cont_port, _wr);

  // Set the Ststus Pins to Input
  cbi(*_cont_ddr, _rxf);
  cbi(*_cont_ddr, _txe);
  
  // Enable the Pull-ups on the Status Pins
  sbi(*_cont_port, _rxf); 
  sbi(*_cont_port, _txe);

  _peekBuf = -1;
}

void FTDI_FT245::end()
{
}

int FTDI_FT245::available(void)
{
  if(_peekBuf != -1)
    return 1;
    
  return (*_cont_pin & (1<<_rxf)) ? 0 : 1;
}

int FTDI_FT245::peek(void)
{
  if(_peekBuf != -1)
    return _peekBuf;

  _peekBuf = read();

  return _peekBuf;
}

int FTDI_FT245::read(void)
{
  if(_peekBuf != -1)
  {
    int retVal = _peekBuf;
    _peekBuf = -1;
    return retVal;
  }

  // check state of RXF line
  // RXF = L  => Data in fifo ready to be read
  // RXF = H  => No data, or not ready
  if(*_cont_pin & (1<<_rxf))
    return -1;

  else
  {
    *_data_ddr = 0x00;
  
    cbi(*_cont_port, _rd);
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    
    unsigned char c = *_data_pin;
    sbi(*_cont_port, _rd);
    
    return c;
  }
}

void FTDI_FT245::flush()
{
}

size_t FTDI_FT245::write(uint8_t c)
{
  // check state of TXE line
  // TXE = L  => Tx fifo is ready to be written
  // TXE = H  => Tx fifo full or not ready
  while(*_cont_pin & (1<<_txe))
    ;	

  sbi(*_cont_port, _wr);
  *_data_ddr = 0xFF;
  *_data_port = c;
  
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  
  cbi(*_cont_port, _wr);
  *_data_ddr = 0x00;  // Put Port DDR Back to Input
  *_data_port = 0xFF;  // Enable The Pull-Ups

  return 1;
}

FTDI_FT245::operator bool() {
	return true;
}
