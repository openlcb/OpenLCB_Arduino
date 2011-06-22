#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <WProgram.h>

class Interface
{
 public:
  virtual void Display(void) {}
  virtual void DisplayMenu(void) {}
  virtual void ProcessKey(unsigned short key) {}
  virtual void ProcessMenuKey(unsigned short key) {}
  virtual void ProcessRepeatKey(unsigned short key) {}
};

#endif
