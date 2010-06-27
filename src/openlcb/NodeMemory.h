#ifndef NodeMemory_h
#define NodeMemory_h

/**
 * Class for persisting node memory
 * in a non-volative memory, e.g. EEPROM.
 *
 * This class loads from and stores to EEPROM
 * a NodeID and two arrays of Events; it's up to you 
 * to store everything else.
 * 
 * The memory can either be 
 * blank, needing complete initialization or
 * OK at startup. Call setup(...) with a valid NodeID
 * to either load or create EventIDs.
 * 
 * If you change those, call store(...) when done.
 * 
 * When you "reset" the memory, you're 
 * putting _new_ unique EventIDs in place.
 *
 * TODO: Add a "dirty" bit to make store logic easier for external code?
 *
 */
#include <stdint.h>

class NodeID;
class Event;
class EventID;

class NodeMemory {
  public:

  /**
   * Define starting address in EEPROM
   */
  NodeMemory(int startAddress);  // doesn't do anything
  
  /**
   * Make sure ready to go.  NodeID should have a default
   * value already in case this is the first time.
   */
  void setup(NodeID* nid, Event* c, int nC, Event* p, int nP);
  
  /*
   * Move to a completely new set of values, e.g. a "default" reset
   * for OpenLCB. NodeID is not changed.
   */
  void reset(NodeID* nid, Event* c, int nC, Event* p, int nP); 

  /** 
   * For debug and test, this forces the next
   * setup() to do a complete reload from initial
   * event values
   */
  void forceInit();
    
  /*
   * Put modified contents back into memory.
   */
  void store(NodeID* nid, Event* c, int nC, Event* p, int nP); 
  
  /*
   * Get a new, forever unique EventID and put in 
   * given EventID location. Does not do a EEPROM store,
   * which must be done separately. unique ID build using
   * this node's nodeID. 
   */
  void setToNewEventID(NodeID* nodeID, EventID* eventID);
  
  private:
  bool checkOK(); // check if memory tag in place
  void writeByte(int address, uint8_t value); // write only if needed
  
  int startAddress; // address of 1st byte in EEPROM
  int count; // count of EventIDs provided to date
};

#endif
