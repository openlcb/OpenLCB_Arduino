#ifndef __LOCOSELECTDISPLAY_H__
#define __LOCOSELECTDISPLAY_H__

#include "Throttle.h"
#include "Interface.h"
#include <OLCB_Link.h>

class LocoSelectDisplay : public Interface
{
 public:
  void init(OLCB_Link *link)
  {
    _address = 0;
    OLCB_NodeID nid(2,1,13,0,0,3);
    for(int i = 0; i < 3; ++i)
    {
      nid.set(2,1,13,0,0,3+i); //this is crap.
      _locos[i].setLink(link);
      _locos[i].setNID(&nid);
    }
  }
  
  boolean ready(void)
  {
    for(int i = 0; i < 3; ++i)
    {
//      Serial.print(_locos[i].NID->alias, DEC);
//      Serial.print(" ");
      if(!_locos[i].NID->alias)
      {
//        Serial.println();
        return false;
      }
    }
//    Serial.println();
    return true;
  }
  
  void Display(void);
  void DisplayMenu(void);
  void ProcessKey(unsigned short key);
  void ProcessMenuKey(unsigned short key);
 private:
  unsigned int _address;
  Throttle _locos[3];
};

#endif
