// Alarm Library
#ifndef Alarm_h
#define Alarm_h

class Alarm {
  public:
    
      Alarm(long p);
      void set(long p);
      bool again(long period);
      bool check();
  private:
      long alarm;
      bool triggered;
};
#endif

/*  Examples:
  Alarm a = Alarm(3000);    // execute in 3s
  Alarm b = Alarm(1000);    // excute in 1s 
  
  if(a.again(3000)) {doSomething(); b.set(5000);}
  if(b.check()) doSomethingElse();
*/

