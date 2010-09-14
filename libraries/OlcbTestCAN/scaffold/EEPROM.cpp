#include "WConstants.h"
#include "EEPROM.h"
#include <stdio.h>

#include "Event.h"

uint8_t EEPROMClass::read(int address)
{
        return array[address];
}

void EEPROMClass::write(int address, uint8_t value)
{
        array[address]=value;
}

void EEPROMClass::dump() {
    for (int i = 0; i <= 3; i++) printf("%2x ", array[i]); printf("\n");
    for (int i = 4; i <= 5; i++) printf("%2x ", array[i]); printf("\n");
    for (int i = 6; i <= 11; i++) printf("%2x ", array[i]); printf("\n");

    int j = 12;
    for (int k = 0; k<4; k++) {
        for (int i = 0; i<sizeof(Event); i++) printf("%2x ", array[j++]); printf("\n");
    }
    printf("\n");
}

EEPROMClass EEPROM;
