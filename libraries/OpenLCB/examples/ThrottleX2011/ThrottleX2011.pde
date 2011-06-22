#include <OLCB_AliasCache.h>
#include <OLCB_Buffer.h>
#include <OLCB_CAN_Buffer.h>
#include <OLCB_CAN_Link.h>
#include <OLCB_Datagram.h>
#include <OLCB_Datagram_Handler.h>
#include <OLCB_Event.h>
#include <OLCB_EventID.h>
#include <OLCB_Handler.h>
#include <OLCB_Link.h>
#include <OLCB_NodeID.h>
#include <OLCB_Stream.h>

#include <can.h>

#include "IBridge.h"
#include "Throttle.h"
#include "PCD8544.h"

// The dimensions of the LCD (in pixels)...
static const byte LCD_WIDTH = 84;
static const byte LCD_HEIGHT = 48;

PCD8544 global_lcd = PCD8544(8, 9, 10, 12, 11);
OLCB_NodeID nid(5,2,1,2,0,1);
OLCB_CAN_Link link(&nid);

Throttle *global_throttle;
unsigned short global_state;

#include "Globals.h"
#include "LocoSelectDisplay.h"
#include "MainDisplay.h"

MainDisplay mainDisplay;
LocoSelectDisplay locoSelectDisplay;
Interface *interface;

void handleDisplay(void)
{
  interface->Display();
  interface->DisplayMenu();
  global_lcd.display();
}

void handleKey(unsigned char key)
{
  if(key == 8 || key == 12 || key == 16)
  {
    interface->ProcessMenuKey(key);
    return;
  }
  
  interface->ProcessKey(key);
}

void handleRepeatKey(unsigned char key)
{
  interface->ProcessRepeatKey(key);
}

/***********************************************************************/
unsigned char key, old_key;

void setup() {
  global_lcd.init();

  // turn all the pixels on (a handy test)
  //global_lcd.command(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYALLON);
  //delay(500);
  // back to normal
  global_lcd.command(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);

  // show splashscreen
  global_lcd.display();
  
  //draw a Railstars logo
//  global_lcd.setCursor(0, 0);
//  global_lcd.drawProgmemBitmap(railstars_logo, RAILSTARS_WIDTH, RAILSTARS_HEIGHT);

//  Serial.begin(115200);
//  Serial.println("RAILSTARS");

  IBridge_GPIO_Config();
  link.initialize();
  
  locoSelectDisplay.init((OLCB_Link*)&link);
  
  while(!locoSelectDisplay.ready())
    link.update();

  global_state = DISP_LOCO_SELECT;
  interface = &locoSelectDisplay;

  key = old_key = 0;

  global_lcd.clear();
  handleDisplay();
}

void loop() { 
  link.update();

  handleDisplay();

  key = IBridge_Read_Key();
  if(key)
  {
    if(key != old_key)
    {
      handleKey(key);
    }
    else
    {
      handleRepeatKey(key);
    }
  }
    

  //update the state
  if(global_state == DISP_MAIN)
    interface = &mainDisplay;
  else if(global_state == DISP_LOCO_SELECT)
    interface = &locoSelectDisplay;

  if(key != old_key)
  {
    delay(20);
  }
  
  old_key = key;
}


/* EOF - TempSensor.pde */

