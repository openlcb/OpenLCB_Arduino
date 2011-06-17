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

void OLCB_Link::removeHandler(OLCB_Handler *handler)
{
  if(handler == _handlers) //if it's at the start
  {
    _handlers = handler->next;
    return;
  }
  //otherwise, find it in the list
  
  OLCB_Handler *iter = _handlers;
  while((iter != NULL) && (iter->next != handler))
  {
    iter = iter->next;
  }
  //here, either we've reached the end of the list, or
  //iter->next == handler
  if(iter)
  {
    iter->next = iter->next->next;
  }
}
