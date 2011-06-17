#include "Globals.h"
#include "LocoSelectDisplay.h"

void LocoSelectDisplay::DisplayMenu(void)
{
    for(int i = 0; i < 3; ++i)
    {
      if(_locos[i].hasAddress()) //if it's been assigned
      {
        global_lcd.drawstring(i*30,5," ");
      }
      else
      {
        global_lcd.drawstring(i*30,5,"---- ");
      }
    }
}

void LocoSelectDisplay::ProcessMenuKey(unsigned short key)
{
  //Two possibilities: If address = 0, the user didn't enter an address. Use the address already assigned to the menu key
  //If address != 0 the user did enter an address. Assign that address to the selected menu key.
  unsigned short i = 0;
  if(key == 16)
    i = 0;
  else if (key == 12)
    i = 1;
  else if (key == 8)
    i = 2;
  else //error!
    return;
  if(_address && (_address < 10000)) //an address was entered
  {
    Serial.print("Got a new address: ");
    Serial.println(_address,DEC);
    _locos[i].setAddress(_address);
  }
  else //no address was entered
  {
    Serial.println("no address entered!");
    if(!_locos[i].hasAddress()) //nothing is in this slot
    {
      return;
    }
  }
  _address = 0;
  global_throttle = &_locos[i];
  //go to main screen
  global_lcd.clear();
  global_state = DISP_MAIN;
  //clear screen
  //Serial.clear();
  delay(50);
}

void LocoSelectDisplay::Display(void)
{
//   Serial.setCursor(1, 1);
   global_lcd.drawstring(6,1,"Address: ");
//   Serial.setCursor(6, 2);
   String add(_address, DEC);
   global_lcd.drawstring(12,2,add);
   global_lcd.drawstring(12+(6*add.length()), 2, "    ");
}

void LocoSelectDisplay::ProcessKey(unsigned short key)
{
  unsigned short val = 99;  
  switch(key)
  {
    case 3: //backspace!
      _address = (unsigned short)(_address / 10); //back it up! Does this do integer division correctly?
      Serial.println(_address);
      break;
    case 4: //cancel
      _address = 0;
      global_lcd.clear();
      global_state = DISP_MAIN;
      //Serial.clear();
      delay(10);
      return;
    case 2:
      break;

    case 1: //0
      val = 0;
      break;
    case 0xD:
      val = 1;
      break;
    case 9:
      val = 2;
      break;
    case 5:
      val = 3;
      break;
    case 0xE:
      val = 4;
      break;
    case 0xA:
      val = 5;
      break;
    case 6:
      val = 6;
      break;
    case 0xF:
      val = 7;
      break;
    case 0xB:
      val = 8;
      break;
    case 7:
      val = 9;
      break;
  }

  //Figure out what to do with it.
  if((val != 99) && (_address*10 < 10000)) //if a numeric key was pressed
  {
    _address *= 10;
    _address += val;
    Serial.println(_address);
  }

  delay(100);
}
