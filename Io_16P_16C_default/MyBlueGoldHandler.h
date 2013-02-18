#ifndef __MYBLUEGOLDHANDLER_H__
#define __MYBLUEGOLDHANDLER_H__

#include <ButtonLed.h>

#include "OLCB_Virtual_Node.h"
#include "MyEventHandler.h"

extern ButtonLed blue; // button on pin 48
extern ButtonLed gold; // button on pin 49

#define UNREADY_BLINK 0xFF00FFL
#define READY_BLINK   0x1L

#define BG_IDLE  0x00
#define BG_LEARN 0x08
#define BG_TEACH 0x80

/**
 * Taken from the OlcbArduinoCAN implementation.
 *
 * Class for Blue/Gold configuration.
 * Works with a PCE instance to do the actual operations.
 * Takes an array of buttons and a corresponding array of patterns
 * to blink for each event producer/consumer to be programmed
 *
 * 1) Push the blue button to light blue and start indexing through event slots.
 *    At each index, a button is given a provided pattern to indicate what
 *    the event slot does. After cycling through, blue goes out.
 * 2) Press gold to select the current slot to learn. Blue goes out and 
 *    you can start again with another if desired.
 * 3) To teach from an event slot, start by pressing gold. Gold comes on.
 * 4) Press blue to select the appropriate event slot to teach. Press gold
 *    again to send the message and reset to start.
 *
 * The gold LED is used to indicate board health.  Before init complete, it's
 * slow blinking 50% on and 50% off.  After init complete, it winks on periodically.
 *
 * A long (>3 seconds) press of just the blue button erases any learn nominations made.
 *
 * A long (>5 seconds) press of blue _and_ gold is a factory reset.
 * Pressing both at the same time for less than 5 seconds sends an ident message.
 *
 */


/* modifications to the above:
 for teaching/learning producers: User must set teach or learn mode as above, then press the input button: Once to select "on" and a second time for "off"
 	TODO this method is reasonable for pushbuttons, but does it work for DPDTs?
 for teaching/learning cosumers: User uses gold button as above to select the output. Notice that this is not a particularly good method if there are turnouts connected to the outputs?
 */

class MyBlueGoldHandler: 
public OLCB_Virtual_Node
{
public:

  void update(void);
  void create(OLCB_Link *link, OLCB_NodeID *nid, MyEventHandler *eventHandler);


protected:
private:
  uint8_t _state;
  bool _started;
  int8_t _index;
  int8_t _input_index;
  bool _last_blue;
  bool _last_gold;
  uint8_t _last_double;
  uint8_t _double_state;
  bool _blue_pressed;
  bool _gold_pressed;
  uint8_t _double_press;

  uint8_t _input_pressed;
  uint8_t _last_input;
  uint8_t _input_count;
  MyEventHandler* _event_handler; //for teaching and learning from.
  uint8_t *_input_buttons; //we know there are 8 of them.

  void sendIdent();
  void factoryReset();  // ToDo: better name!  Not really a true "factory reset"
  void moveToIdle(bool reset=false);
};

#endif

