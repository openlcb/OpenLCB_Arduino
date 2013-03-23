#include <Arduino.h>


// ui for changing a time
enum {uiTimeIdle, ui10Hour, ui1Hour, ui1Min, ui10Min} uiTimeState;
bool setTime(struct time_s *t) {
  uint8_t h10 = t->hour/10;
  uint8_t h1 = t->hour%10;
  uint8_t m10 = t->minute/10;
  uint8_t m1 = t->minute%10;
  //Serial.println(".set.time.");
  switch (uiTimeState) {
  case ui10Hour:
    if(blue.unique()) { if((h10+=1)>3) 
      h10=0;
      Serial.print(h10*10+h1);Serial.print(":");Serial.println(m10*10+m1);
    }
    if(gold.unique()) uiTimeState=ui1Hour;
    break;  
  case ui1Hour:
    if(blue.unique()) {
      if((h1+=1)>9 || (h10*10+h1)>23) h1=0;
      Serial.print(h10*10+h1);Serial.print(":");Serial.println(m10*10+m1);
    }
    if(gold.unique()) uiTimeState=ui10Min;
    break;  
  case ui10Min:
    if(blue.unique()) {
      if((m10+=1)>5) m10=0;
      Serial.print(h10*10+h1);Serial.print(":");Serial.println(m10*10+m1);
    }
    if(gold.unique()) uiTimeState=ui1Min;
    break;  
  case ui1Min:
    if(blue.unique()) {
      if((m1+=1)>9) m1=0;
      Serial.print(h10*10+h1);Serial.print(":");Serial.println(m10*10+m1);
    }
    if(gold.unique()) {
      Serial.print(h10*10+h1);Serial.print(":");Serial.println(m10*10+m1);
      uiTimeState = uiTimeIdle; 
      return true;
    }
    break; 
  }
  t->hour  = h10*10+h1;
  t->minute= m10*10+m1; 
  return false;
}

enum {uiRateIdle, uiRateOnce, ui100Rate, ui10Rate, ui1Rate, uiFracRate} uiRateState;
bool setRate(int *r) {
  // r is xxxxxxxxxxxxxx.xx
  byte r100 = *r/400;
  byte r10 = *r%400/40;
  byte r1 = *r%40/4;
  byte rq = *r&0x03;
  //Serial.println(".set.rate.");
  switch (uiRateState) {
  case uiRateOnce:
    Serial.println("Once:");
    Serial.println(r100,DEC);
    Serial.println(r10,DEC);
    Serial.println(r1,DEC);
    Serial.println(rq,DEC);
    uiRateState=ui100Rate;
    break;
  case ui100Rate:
    if(blue.unique()) { 
      if((r100+=1)>9) r100=0;
    Serial.println(r100,DEC);
    Serial.println(r10,DEC);
    Serial.println(r1,DEC);
    Serial.println(rq,DEC);
      Serial.println((r100*400+r10*40+r1*4+rq)/4.0);
    }
    if(gold.unique()) uiRateState=ui10Rate;
    break;  
  case ui10Rate:
    if(blue.unique()) {
      if((r10+=1)>9) r10=0;
    Serial.println(r100,DEC);
    Serial.println(r10,DEC);
    Serial.println(r1,DEC);
    Serial.println(rq,DEC);
      Serial.println((r100*400+r10*40+r1*4+rq)/4.0);
    }
    if(gold.unique()) uiRateState=ui1Rate;
    break;  
  case ui1Rate:
    if(blue.unique()) {
      if((r1+=1)>9) r1=0;
    Serial.println(r100,DEC);
    Serial.println(r10,DEC);
    Serial.println(r1,DEC);
    Serial.println(rq,DEC);
      Serial.println((r100*400+r10*40+r1*4+rq)/4.0);
    }
    if(gold.unique()) uiRateState=uiFracRate;
    break;  
  case uiFracRate:
    if(blue.unique()) {
      if((rq+=1)>3) rq=0;
    Serial.println(r100,DEC);
    Serial.println(r10,DEC);
    Serial.println(r1,DEC);
    Serial.println(rq,DEC);
      Serial.println((r100*400+r10*40+r1*4+rq)/4.0);
    }
    if(gold.unique()) {
    Serial.println(r100,DEC);
    Serial.println(r10,DEC);
    Serial.println(r1,DEC);
    Serial.println(rq,DEC);
      Serial.println((r100*400+r10*40+r1*4+rq)/4.0);
      uiRateState=uiRateIdle;
      *r = r100*400+r10*40+r1*4+rq;
      cPeriod = 240000 / *r;
      store();
      return true;
    }
    break; 
  }
  *r = r100*400+r10*40+r1*4+rq;
  return false;
}
/*
// ui for changing the rate
#define nRates 21
uint8_t rates[nRates] = {1,2,3,4,5,10,15,20,25,30,40,45,50,60,75,100,120,150,180,200,250}; 
bool setRate(uint8_t *r) {
  //Serial.println("setRate");
  if(blue.unique()) {
    if(++*r>=nRates) *r=0;
    Serial.println(rates[*r],DEC);
  }
  if(gold.unique()) return true;
  return false;
}
*/
enum{uiIdle, uiSet, uiSet2, uiSet3, uiNom, uiNom2, uiTeach, uiTeach2, uiTeach3} uiMode;
enum{uiChooseIdle, uiSetTime, uiTime, uiRate, uiAlarm, uiAlarm2 } uiChooseState;
uint8_t alarm;
void uiReset() {
  uiMode = uiIdle;
  uiChooseState=uiChooseIdle;
  uiTimeState=uiTimeIdle;
  alarm=0;
  gold.on(0L);  
  blue.on(0L); Serial.println("~BG"); 
}
uint8_t uiChoose() {
  switch (uiChooseState) {
  case uiChooseIdle: 
    if(blue.unique()) {uiChooseState=uiSetTime; } //                         Serial.println("Time"); }
    if(gold.unique()) uiReset(); return 0;
    break;
  case uiSetTime: 
    if(blue.unique()) {uiChooseState=uiTime; } //                         Serial.println("Rate"); }
    if(gold.unique()) return 1;
    break;
  case uiTime: 
    if(blue.unique()) {uiChooseState=uiRate; } //                         Serial.println("Rate"); }
    if(gold.unique()) return 2;
    break;
  case uiRate:
    if(blue.unique()) { alarm=0; uiChooseState=uiAlarm; } //               Serial.println("Alarm:0");}
    if(gold.unique()) return 3;
    break;
  case uiAlarm:
    if(blue.unique()) { if((++alarm)>=nAlarms) { uiReset(); return 0; }} //  Serial.println(alarm);}
    if(gold.unique()) { return alarm+4; }
    break;
  }
  return 0;
}

uint8_t uiIndex;
bool identSentF, factoryResetF, nomCancelF;
int uiGeneric() {
  // generic b/g functionality

  if ( (blue.state) && (gold.state) && (blue.duration > 5000) && (gold.duration > 5000) ) { 
    if(!factoryResetF) {
      factoryResetF=true; 
      //Serial.println("factoryReset()"); 
      return 1;
    }
  } else factoryResetF=false;
  
  if ( (blue.state) && (!gold.state) && (blue.duration > 3000) ) {
    if(!nomCancelF) {
      blue.on(0L); 
      nomCancelF=true;
      for (int i = 0; i < nEvents; i++) pce.markToLearn(i, false);
      return 2;
    }
  } else nomCancelF=false;
  
  if(blue.state && gold.state) { 
    if(!identSentF) { 
      Serial.println("sendIdent()"); 
      identSentF=true; 
      return 3;
    }
  } else identSentF=false;
  
  return 0;
}  

uint8_t uiCheck() {
  if(uiGeneric()) {uiReset(); return 0;}
  // ui for choosing to set, nominate, or teach
  // (b): set (g) (b)+: time, rate, alarm0..n (g) ...
  // (b): nom (g) (b)+: alarm0..n (g)
  // (g): teach (b)+: time, rate, alarm0..n (g)
  switch (uiMode) {
  case uiIdle:
    if(blue.unique()) { uiMode=uiSet; }                       //        Serial.println("Set:");}
    if(gold.unique()) {gold.on(~0L); uiMode=uiTeach; }        //        Serial.println("Teach:"); }
    break;
  case uiSet:
    if(blue.unique()) {blue.on(~0L); uiMode=uiNom; }          //        Serial.println("Nom:"); }
    if(gold.unique()) {uiMode=uiSet2; uiChooseState=uiTime; } //        Serial.println("time"); }
    break;
  case uiSet2:
    if(uiIndex=uiChoose()) 
     {uiMode=uiSet3; uiTimeState=ui10Hour; uiRateState=uiRateOnce; zTime.hour=0; zTime.minute=0; } // Serial.print("index=");Serial.println(uiIndex,DEC); }
    break;
  case uiSet3:
    if(uiIndex==2) { if(setTime(&zTime)) { cTime=zTime; return uiIndex; } }
    if(uiIndex==3) { if(setRate(&e.cRate)) { return uiIndex; } }
    if(uiIndex>3)  { if(setTime(&e.alarms[uiIndex-4])) { store(); return uiIndex; } }
    break;
  case uiNom:
    if(blue.unique()) { uiReset(); return 0;}
    if(gold.unique()) {uiMode=uiNom2; uiChooseState=uiAlarm; }
    break;
  case uiNom2:
    if(uiIndex=uiChoose()) {
      //if(uiIndex==2) { Serial.println("markTime"); } //markTime=1;
      //if(uiIndex==3) { Serial.println("markRate"); } //markRate=1;
      if(uiIndex>3)  { Serial.print("markToLearn:"); Serial.println(uiIndex-4); pce.markToLearn(uiIndex-4, true); }
      return uiIndex;
    }
    break;
  case uiTeach:
    if(gold.unique()) {uiReset(); return 0;}
    if(blue.unique()) {uiMode=uiTeach2;uiChooseState=uiSetTime;}
    break;
  case uiTeach2:
    if(uiIndex=uiChoose()) 
     {uiMode=uiTeach3; uiTimeState=ui10Hour; zTime.hour=0; zTime.minute=0; } // Serial.print("index=");Serial.println(uiIndex,DEC); }
    break;
  case uiTeach3:
//      if(uiIndex==1) pce.sendLearn(Event(FC_prefix, cTime.hour, cTime.minute)); // Serial.print("TeachTime:");Serial.print(cTime.hour);Serial.println(cTime.minute); //sendTime();
    if(uiIndex==1) if(setTime(&zTime)) { pce.sendTeach(Event(FC_prefix, zTime.hour+60, zTime.minute)); return uiIndex; } 
    if(uiIndex==2) if(setTime(&zTime)) { pce.sendTeach(Event(FC_prefix, zTime.hour, zTime.minute)); return uiIndex; } 
    if(uiIndex==3) { pce.sendTeach(Event(FC_prefix, 0xFC|(e.cRate>>8), e.cRate)); return uiIndex; } // Serial.println("TeachRate"); //sendRate();
    if(uiIndex>3) if(e.alarms[uiIndex-4].hour<24) {pce.sendTeach(uiIndex-4); return uiIndex;}
    break;
  }
  //showTime();
  blue.process();
  gold.process();
  return 0;
}

