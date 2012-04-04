#ifndef __MYINFOHANDLER_H__
#define __MYINFOHANDLER_H__

#include "OLCB_Virtual_Node.h"

/**
 * A class for generating basic canned messages for PIP and SNIP.
 */

class MyInfoHandler: 
public OLCB_Virtual_Node
{
public:

  void update(void);
  bool handleMessage(OLCB_Buffer *buffer);
  void create(OLCB_Link *link, OLCB_NodeID *nid);

private:
  char _buffer[19];
  int8_t _string_index;
  uint8_t _string_length;
  OLCB_Buffer _reply;
};

#endif

