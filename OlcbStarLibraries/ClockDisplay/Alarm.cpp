// Alarm Library

#include <Arduino.h>
#include "Alarm.h"

void Alarm::set(long p) {
  alarm = millis() + p;
}   

Alarm::Alarm(long p) {
  set(p);
}

bool Alarm::again(long period) {
  long now=millis();
  if(now>=alarm) {
    if(triggered) return false;
    alarm=now+period; 
    triggered=true;
  } else 
    triggered = false;
  return triggered;
}

bool Alarm::check() { 
  return again(0); 
}

/*  Examples:
  Alarm a = Alarm(3000);    // execute in 3s
  Alarm b = Alarm(1000);    // excute in 1s 
  
  if(a.again(3000)) {doSomething(); b.set(5000);}
  if(b.check()) doSomethingElse();
*/

