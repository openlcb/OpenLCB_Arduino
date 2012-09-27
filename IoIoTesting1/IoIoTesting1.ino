#include <Arduino.h>
#include <ButtonLED.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#define NPINS 16

#include <EEPROM.h>
void writeNodeID_Io(uint8_t Addr)  // For Io_16P_16C_default
{
  EEPROM.write(0x0FFA, 0x05);
  EEPROM.write(0x0FFB, 0x02);
  EEPROM.write(0x0FFC, 0x01);
  EEPROM.write(0x0FFD, 0x02);
  EEPROM.write(0x0FFE, 0x02);
  EEPROM.write(0x0FFF, Addr); 
}

void writeNodeID_Olcb(uint8_t Addr)  // For OlcbBasicNode
{
  EEPROM.write(0x00, 0xEE);
  EEPROM.write(0x01, 0x55);
  EEPROM.write(0x02, 0x5E);
  EEPROM.write(0x03, 0xE5);
  EEPROM.write(0x04, 0x00);
  EEPROM.write(0x05, 0x00);
  
  EEPROM.write(0x06, 0x05);
  EEPROM.write(0x07, 0x02);
  EEPROM.write(0x08, 0x01);
  EEPROM.write(0x09, 0x02);
  EEPROM.write(0x0A, 0x02);
  EEPROM.write(0x0B, Addr); 
}


ButtonLed b[50];

long start;
bool once;



void setup() {
   
  // Change NodeAddrLastByte to your Node Address
  uint8_t NodeAddrLastByte = 0x01 ;       // <<<<   CHANGE THIS AND ...
  // First erase the EEPROM
  for(uint16_t i = 0; i < 0x0FFF; i++)
    if( EEPROM.read(i) != 0xFF)
      EEPROM.write(i, 0xFF);

//  Uncomment for the Io Code  
  writeNodeID_Io(NodeAddrLastByte);       // <<<<<<<< CHANGE THIS AND ...  

//  Uncomment for the OlcbBasicNode Code  
//  writeNodeID_Olcb(NodeAddrLastByte);   // <<<<<< CHANGE THIS                 
  
  // set up all pins for action
  for(uint8_t i=0;i<NPINS;i++) {
    b[i].setPinSense(i, LOW);
    b[i].on(0x0000FFFFL);      // blink its LED
  }

    pinMode(49,INPUT);
    pinMode(48,INPUT);

    DDRG |= 0x03;
    b[BLUE].setPinSense(BLUE, LOW);
    b[BLUE].on(0x0000FFFFL);      // blink its LED
    b[GOLD].setPinSense(GOLD, LOW);
    b[GOLD].on(0x00FF00FFL);      // blink its LED
    
    Serial.begin(9600);
    Serial.println(BLUE);
    Serial.println(GOLD);
  start = millis();  
  once = false;
}

void loop() {
  // lopp through each pin
  for(uint8_t i=0; i<NPINS; i++) {
    b[i].process();
  }
/*
  if(once==false) {
    if((millis()-start)>5000) {
        Serial.println(millis());
        for(uint8_t i=0;i<NPINS;i++) {
          b[i].on(~0x0L);      // leds off
        }
        once = true;
    }
  }

  for(uint8_t i=8; i<NPINS; i++) {
    if( b[i].state) {                       // is the button down?
      b[i].on(~b[i].pattern);      // blink its LED
      b[i%8].on(~b[i%8].pattern);      // blink the outputs LED
    } 
  }
*/

  b[BLUE].process();
  b[GOLD].process();
}
