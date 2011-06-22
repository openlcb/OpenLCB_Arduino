#ifndef __MAINDISPLAY_H__
#define __MAINDISPLAY_H__

#include "Interface.h"


class MainDisplay : public Interface
{
 public:
  void Display(void);
  void DisplayMenu(void);
  void ProcessKey(unsigned short key);
  void ProcessMenuKey(unsigned short key);
  void ProcessRepeatKey(unsigned short key);
};

#endif
