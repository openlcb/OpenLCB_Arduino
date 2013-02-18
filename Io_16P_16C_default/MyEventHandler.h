#ifndef __MYEVENTHANDLER_H__
#define __MYEVENTHANDLER_H__

#include "ButtonLed.h"

#include "OLCB_Virtual_Node.h"
#include "OLCB_Event_Handler.h"

extern ButtonLed blue; // button on pin 14
extern ButtonLed gold; // button on pin 15

#ifndef EVENT_POOL_SIZE
#define EVENT_POOL_SIZE 64
#endif


/************************
 * Here is the class for handling PC Event Reports
 ************************/
class MyEventHandler: 
public OLCB_Event_Handler
{
public:
  MyEventHandler(void) : 
  _inhibit(false), _first_check(true), _first_run(true), _inputs(0x00), _dirty(0)
  {
    for(uint8_t i = 0; i < 8; ++i)
      _input_buttons[i] = i+8;
  }

  void initialize(OLCB_Event *events, uint8_t num); //assume offset is 0, uint8_t offset);

  void factoryReset(void);

  void update(void);

  bool consume(uint16_t index);

  uint32_t getLargestAddress(void);
  uint8_t readConfig(uint16_t address, uint8_t length, uint8_t *data);
  void writeConfig(uint16_t address, uint8_t length, uint8_t *data);
  uint8_t *getInputs(void) {
    return _input_buttons;
  }
  void inhibit(void) {
    _inhibit = true;
  }
  void disInhibit(void) {
    _inhibit = false;
  }

protected:
  bool store(void);
  bool load(void);

private:
  bool _inhibit;
  bool _first_check;
  bool _first_run;
  uint8_t _inputs;
  uint8_t _dirty;
  uint8_t _input_buttons[8]; //TODO 
  void firstInitialization(void);
};


#endif //__MYEVENTHANDLER_H__

