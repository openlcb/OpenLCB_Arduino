#include "Globals.h"
#include "MainDisplay.h"

void MainDisplay::DisplayMenu(void)
{
  global_lcd.drawstring(0, 5, "LOCO ");
  global_lcd.drawstring(30, 5, "---- ");
  global_lcd.drawstring(60, 5, "---- ");
}

void MainDisplay::ProcessMenuKey(unsigned short key)
{
  if(key == 16) //go to loco select
  {
    global_state = DISP_LOCO_SELECT;
    global_lcd.clear();
    delay(50);
  }
}

void MainDisplay::Display(void)
{  
  // Print the loco
//  Serial.setCursor(0, 0);
  if(!global_throttle->hasAddress())
  {
    global_lcd.drawstring(0, 1, "No loco.");
    return;
  }


  // Print the address
  global_lcd.drawstring(0, 0, "Loco ");
  global_lcd.drawstring(30, 0, String(global_throttle->getAddress(), DEC));
  
  // Print the speed
  //Serial.setCursor(0, 1);
  global_lcd.drawstring(0,1,"Speed: ");
  unsigned short speed = global_throttle->getSpeed();
  if(speed == 100)
    global_lcd.drawstring(42,1,String(speed, DEC));
  else if(speed >= 10)
  {
    global_lcd.drawstring(42,1," ");
    global_lcd.drawstring(48,1,String(speed, DEC));
  }
  else
  {
    global_lcd.drawstring(42,1,"  ");
    global_lcd.drawstring(54,1,String(speed, DEC));
  }
  global_lcd.drawstring(60,1,"%");

  // Update the throttle display 
  //Serial.setCursor(78, 0);
  //Serial.drawColumn(5, map(speed, 0, 100, 1, 5*8));  // ...clipped to the 0-45C range.
  //Serial.setCursor(79, 0);
  //Serial.drawColumn(5, map(speed, 0, 100, 1, 5*8));  // ...clipped to the 0-45C range.
  //Serial.setCursor(80, 0);
  //Serial.drawColumn(5, map(speed, 0, 100, 1, 5*8));  // ...clipped to the 0-45C range.
  //Serial.setCursor(81, 0);
  //Serial.drawColumn(5, map(speed, 0, 100, 1, 5*8));  // ...clipped to the 0-45C range.
  //Serial.setCursor(82, 0);
  //Serial.drawColumn(5, map(speed, 0, 100, 1, 5*8));  // ...clipped to the 0-45C range.

  // update the function display
  //TODO
}

void MainDisplay::ProcessKey(unsigned short key)
{
  // keypad layout:
  // G C 8 4
  // F B 7 3
  // E A 6 2
  // D 9 5 1
  unsigned short func = 0;
  unsigned short speed = 0;
  boolean direction = true;
//  Serial.print("Keypress! ");
//  Serial.println(key,HEX);
  switch(key)
  {
    //speed keys
  case 2: //decrease speed
    speed = global_throttle->getSpeed();
    direction = global_throttle->getDirection();
    if(speed > 0) global_throttle->setSpeed(speed - 1, direction);
    return;
  case 3: //increase speed
    speed = global_throttle->getSpeed();
    direction = global_throttle->getDirection();
    if(speed < 100) global_throttle->setSpeed(speed + 1, direction);
    return;
  case 4: //direction
    speed = global_throttle->getSpeed();
    direction = global_throttle->getDirection();
    global_throttle->setSpeed(speed, !direction);
    return;

    //function keys:
    //
    // G C 8 4
    // F B 7 3
    // E A 6 2
    // D 9 5 1
    //
    // M M M D
    // 7 8 9 +
    // 4 5 6 -
    // 1 2 3 0

  case 1:
    func = 0;
    break;
  case 0xD:
    func = 1;
    break;
  case 9:
    func = 2;
    break;
  case 5:
    func = 3;
    break;
  case 0xE:
    func = 4;
    break;
  case 0xA:
    func = 5;
    break;
  case 6:
    func = 6;
    break;
  case 0xF:
    func = 7;
    break;
  case 0xB:
    func = 8;
    break;
  case 7:
    func = 9;
    break;
  default:  //error!
    return;
  }
  boolean func_val = global_throttle->getFunction(func);
  global_throttle->setFunction(func, !func_val);
}

