//
// Dummy implementation of the Arduino EEPROM
// library

#ifndef EEPROM_h
#define EEPROM_h

#include <inttypes.h>

class EEPROMClass
{
  public:
    uint8_t read(int);
    void write(int, uint8_t);
    void dump();
    
    private:
    uint8_t array[128];
};

extern EEPROMClass EEPROM;

#endif
