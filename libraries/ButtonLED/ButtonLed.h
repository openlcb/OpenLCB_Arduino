#ifndef ButtonLED_h
#define ButtonLED_h

// ButtonLED class
// By David Harris   Copyright 2010
//
// Define a button: buttonLed b0(pin)
// Periodically call .process()
//
// .state is the current state, as would-be read from the pin
// .duration is how long the button has been in its current state in msec
// .lastDuration is how long it was in the previous state, in msec
//
// .on(pattern) will drive the LED from the given pattern, LSB first
//     0 bits in pattern are off, 1 bits are on
//     each bit shows for 64 msec, aligned across all ButtonLed instances
//  full off:             0x0L
//  full on:             ~0x0L
//  slowest flash: 0x0000FFFFL
//  fastest flash: 0x55555555L
//  blink:         0x00000003L
//  double blink:  0x00000033L
//  wink:          0xFFFFFFFEL

// Hardware is e.g:
//  +5V---R1---LED>|----pin----R2----button---gnd
// where R1 is 0.5-1k, R2 is >0.5k.
// Values of 500 and 1k work well for cheap LEDs. 
// For high intensity LEDs, R1 should perhaps be 1k.  

// #include "OlcbUtil.h"
#include <Arduino.h>

class ButtonLed {
  private:
    long timeOfLastChange;    // time at whihc the button last changed
    int lastLEDPeriod;        // last period processed for LED 
    int lastButtonPeriod;     // last period processed for button
    bool lastState;           // state of button in last scan
    bool newState;            // state of button this scan
    void init(uint8_t s);     // init
	bool lastUState;
  public:
    uint16_t sense;            // active sense of button/LED 
    long pattern;             // current output drive pattern (rotates)
    long duration;            // how long the button has been in its current state in msec
    long lastDuration;        // how long it was in the previous state, in msec
    uint8_t pin;              // connected pin number
    uint8_t once;             // mask for single blink
    bool ledState;            // current output drive state
    bool state;               // current button state

    ButtonLed(uint8_t pin, uint8_t sense);  // associate pin and sense for active state
    ButtonLed(uint8_t pin);                 // associate pin, default sense=HIGH
	ButtonLed();
	void setPinSense(uint8_t p, uint8_t s);
    void on(long pattern);                  // set the output repeating pattern
    void blink(uint8_t pattern);            // set the output one-time blink pattern
    void process();                         // call periodically
	bool unique();
};

#endif 
