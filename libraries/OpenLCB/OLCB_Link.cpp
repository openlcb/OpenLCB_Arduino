#include "OLCB_Link.h"

void OLCB_Link::update(void)//called repeatedly
{
  //call all our handler's update functions
  OLCB_Handler *iter = _handlers;
  while(iter)
  {
    iter->update();
    iter = iter->next;
  }
}
  
void OLCB_Link::addHandler(OLCB_Handler *handler)
{
//    Serial.println("OLCB_Link::addHandler: registering handler");
  handler->next = _handlers;
  _handlers = handler;
}