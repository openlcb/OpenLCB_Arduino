/****************************************************************************
 *  FT245.h - FTDI library
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

#ifndef FT245_h
#define FT245_h

#include <inttypes.h>

#include "Stream.h"

class FTDI_FT245 : public Stream
{
  private:
    volatile uint8_t *_data_ddr;
    volatile uint8_t *_data_port;
    volatile uint8_t *_data_pin;
    volatile uint8_t *_cont_ddr;
    volatile uint8_t *_cont_port;
    volatile uint8_t *_cont_pin;
    uint8_t _rd;
    uint8_t _wr;
    uint8_t _rxf;
    uint8_t _txe;
    int _peekBuf;
  
  public:
    FTDI_FT245(
      volatile uint8_t *data_ddr, volatile uint8_t *data_port, volatile uint8_t *data_pin,
      volatile uint8_t *cont_ddr, volatile uint8_t *cont_port, volatile uint8_t *cont_pin,
      uint8_t rd, uint8_t wr, uint8_t rxf, uint8_t txe );
    void begin();
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool();
};

#endif

