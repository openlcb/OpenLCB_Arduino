/*
  FT245.h - FTDI library
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

