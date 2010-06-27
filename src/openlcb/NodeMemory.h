#ifndef NodeMemory_h
#define NodeMemory_h

/**
 * Class for persisting node memory
 * in a non-volative memory, e.g. EEPROM.
 *
 * This class doesn't take any space, it just
 * organizes code.  It stores a NodeID and 
 * two arrays of Events; it's up to you to store 
 * everything else.
 * 
 * The memory can either be 
 * blank, needing complete initialization or
 * OK at startup.
 * 
 * When you "reset" the memory, you're 
 * putting _new_ unique EventIDs in place.
 *
 */
#include <stdint.h>

class NodeID;
class Event;

class NodeMemory {
  public:

  NodeMemory();  // doesn't do anything
  
  /**
   * Make sure ready to go.  NodeID should have a default
   * value already in case this is the first time.
   */
  void setup(int address, NodeID* nid, Event* c, int nC, Event* p, int nP);
  
  /*
   * Move to a completely new set of values, e.g. a factory reset
   * for OpenLCB. NodeID is not changed.
   */
  void reset(int address, NodeID* nid, Event* c, int nC, Event* p, int nP); 

  /*
   * Put modified contents back into memory.
   */
  void store(int address, NodeID* nid, Event* c, int nC, Event* p, int nP); 
  
  private:
  bool checkOK(int address); // check if memory tag in place
  void writeByte(int address, uint8_t value); // write only if needed
  
};

#endif
