//#include <vLCmini.h>
//#include "Alarm.h"
//Alarm blinkAlarm = Alarm(750);

/*
 * (gold) Teach
 *  (blue) Time
 *  (blue) SetTime
 *  (blue) SetRate
 *  (blue) Alarm0
 *  ...
 *  (blue) Alarm7
 *  (blue) Run
 */



void display24h(struct time_s t) {
  if(t.hour>23) {
    lcd.print("**:**");
    return;
  }
  if(t.hour<10) lcd.print("0");
  lcd.print(t.hour,DEC);
  lcd.print(":");
  if(t.minute<10) lcd.print("0");
  lcd.print(t.minute,DEC);
}  

void timeCursor() {
  lcd.cursor();
  int i;
  switch (uiTimeState) {
  case ui10Hour: i=10; break;
  case ui1Hour:  i=11; break;
  case ui10Min:  i=13; break;
  case ui1Min:   i=14; break;
  }
  lcd.setCursor(i,1);
}

void displayTime() {
  // +0123456789012345+
  // |00:00 Rate:000  |
  // |Set   Alarm1    |
  // +----------------+
  lcd.clear();
  lcd.setCursor(0,0);
  display24h(cTime);
  lcd.print(" 1:");
  lcd.print(e.cRate/4.0);
  lcd.setCursor(0,1);
  switch (uiMode) {
  case uiSet: case uiSet2: case uiSet3: 
    lcd.print("Set "); break;
  case uiNom: case uiNom2: 
    lcd.print("Nom "); 
    for(int i=0; i<nAlarms; i++) (pce.isMarkedToLearn(i) ? lcd.print(i,HEX) : lcd.print(".") );
    break;
  case uiTeach: case uiTeach2: case uiTeach3: lcd.print("Tch "); break;
  }
  switch (uiChooseState) {
  case uiChooseIdle:
    if(uiMode==uiIdle) {
      lcd.print("Run  ");
      for(int i=0; i<nAlarms; i++) (e.alarms[i].hour<24 ? lcd.print(i,HEX) : lcd.print(".") );
    }
    break;
  case uiSetTime:
    lcd.print("STime "); 
    display24h(zTime);
    timeCursor();
    break;
  case uiTime: 
    lcd.print(" Time "); 
    if(uiMode==uiSet3 || uiMode==uiTeach3) {
      display24h(zTime);
    } else {
      display24h(cTime);
    }
    timeCursor();
    break;
  case uiRate: 
    lcd.print("SRate "); 
    //lcd.print(" "); 
    if(e.cRate<400) lcd.print(0);
    if(e.cRate<40) lcd.print(0);
    lcd.print(e.cRate/4.0); 
    int i;
    switch(uiRateState) {
    case ui100Rate: i=9; break;
    case ui10Rate: i=10; break;
    case ui1Rate: i=11; break;
    case uiFracRate: i=13; break;
    }
    lcd.cursor();
    lcd.setCursor(i,1);
    break;
  case uiAlarm: case uiAlarm2: 
    lcd.setCursor(4,1);
    lcd.print("Alrm"); lcd.print(alarm,DEC); lcd.print(" ");
    display24h(e.alarms[alarm]);
    timeCursor();
    break;
  }
  if(uiTimeState==uiTimeIdle) lcd.noCursor();
  
  if(identSentF)    { lcd.setCursor(0,1); lcd.print("SendIdend"); }
  if(nomCancelF)    { lcd.setCursor(0,1); lcd.print("Noms cancelled"); }
  if(factoryResetF) { lcd.setCursor(0,1); lcd.print("Factory Reset"); }
}

