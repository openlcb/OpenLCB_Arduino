#include "OLCB_Handler.h"

void OLCB_Handler::setNID(OLCB_NodeID *newNID)
{
  NID = (OLCB_NodeID*)malloc(sizeof(OLCB_NodeID));
  memcpy(NID,newNID, sizeof(OLCB_NodeID));
}

void OLCB_Handler::setLink(OLCB_Link *newLink)
{
  _link = newLink;
  NID = _link->getNodeID();
  _link->addHandler(this);
}
