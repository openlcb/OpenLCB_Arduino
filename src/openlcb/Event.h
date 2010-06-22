#ifndef Event_h
#define Event_h

#include "EventID.h"

class Event : public EventID {
  public: 

  Event() {
  }
  
  Event(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7) 
      : EventID(b0, b1, b2, b3, b4, b5, b6, b7){
  }
  
  bool equals(Event* n) {
    return  EventID::equals(n);  // just check ID for now
  }
  
  /**
   * Check to see if this object is equal
   * to any in an array of Events
   */
  Event* findEventInArray(Event* array, int len) {
      for (int i = 0; i<len; i++) {
          if (equals(array+i)) return array+i;
      }
      return 0;
  }
  
  int findIndexInArray(Event* array, int len) {
      for (int i = 0; i<len; i++) {
          if (equals(array+i)) return i;
      }
      return -1;
  }

  // bit mask local flags
  int flags;
};

#endif
