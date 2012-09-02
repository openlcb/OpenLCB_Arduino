#ifndef BP_h
#define BP_h

#include "ButtonLed.h"
#include "OpenLcbCanBuffer.h"
#include "logging.h"

#include "LinkControl.h"
#include "OpenLcbCanBuffer.h"
#include "OpenLcbCan.h"

/**
 * Class for Remote Button Protocol.
 *
 * Takes pointer to Blue and Gold ButtonLeds, plus an array of ButtonLeds to serve
 *
 * check() assumes that address has already been checked, and does not
 * verify it again.
 */

class ButtonLed;
class OpenLcbCanBuffer;

class ButtonProtocol {
  public:

  ButtonProtocol(ButtonLed** buttons, uint8_t nButtons, ButtonLed* blue, ButtonLed* gold);
  bool receivedFrame(OpenLcbCanBuffer* rcv);
  void check();
  void sendUI(char* s); 

  private:

  ButtonLed** buttons;
  uint8_t nButtons;
  ButtonLed* blue;
  ButtonLed* gold;
  uint8_t byt, bit;
  uint8_t indicators[8];
  
};

#endif
