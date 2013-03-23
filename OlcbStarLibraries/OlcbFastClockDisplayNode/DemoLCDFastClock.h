/*
  LiquidCrystal Library - Autoscroll
 
 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the 
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.
 
 This sketch demonstrates the use of the autoscroll()
 and noAutoscroll() functions to make new text scroll or not.
 
 The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe 
 modified 25 July 2009
 by David A. Mellis
 
 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

// include the library code:
#include <LCmini.h>

// initialize the library with the numbers of the interface pins
LCmini lcd(12, 11, 5, 4, 3, 2);

int time;

uint8_t custom_0[] = {0x1f, 0x1f, 0x1f, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c}; // lu upper left corner
uint8_t custom_1[] = {0x1f, 0x1f, 0x1f, 0x07, 0x07, 0x07, 0x07, 0x07}; // ru upper right
uint8_t custom_2[] = {0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1f, 0x1f, 0x1f}; // ll lower left
uint8_t custom_3[] = {0x07, 0x07, 0x07, 0x07, 0x07, 0x1f, 0x1f, 0x1f}; // rl lower right
uint8_t custom_4[] = {0x1f, 0x1f, 0x1f, 0x1c, 0x1c, 0x1f, 0x1f, 0x1f}; // lc left c
uint8_t custom_5[] = {0x1f, 0x1f, 0x1f, 0x07, 0x07, 0x1f, 0x1f, 0x1f}; // rc right c
uint8_t custom_6[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x1f}; // lb lower bar
uint8_t custom_7[] = {0x1f, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00}; // ub upper bar
#define LU 0
#define RU 1
#define LL 2
#define RL 3
#define LC 4
#define RC 5
#define LB 6
#define UB 7
#define BL ' '

// 0 = 0 1 2 3  ul ur ll lr
// 1 = 1 b 3 6  ul bb lr l-
// 2 = 7 5 4 6  ul rc ll lr
// 3 = 7,5,2,3  u- rc l- ll
// 4 = ll lr bb rr
// 5 = lc u- l- rc
// 6 = ul u- lc rc
// 7 = u- ur bb lr
// 8 = lc rc ll lr
// 9 = lc rc l- lr
// : = l- u-

// \4\5 \4\5 \6 \4\5 \4\5 Rate=
// \2\3 \2\3 \7 \2\3 \2\3 1:20
#define DELAY 5
void showTime(int time, int pos) {
  int min = time%60;
  int m10 = min/10;
  int m1 = min%10;
  int hr = time/60%24; 
  int h10 = hr/10;
  int h1 = hr%10;
  static uint8_t dig[][4] = { // upper left, upper right, lower left, lower right
    {LU,RU,LL,RL}, // 0 
    {RU,BL,RL,LB}, // 1
    {UB,RC,LC,LB}, // 2
    {UB,RC,LB,RL}, // 3
    {LL,RL,BL,RU}, // 4
    {LC,UB,LB,RC}, // 5
    {LU,UB,LC,RC}, // 6
    {UB,RU,BL,RU}, // 7
    {LC,RC,LL,RL}, // 8
    {LC,RC,LB,RL}, // 9
  };
  lcd.setCursor(pos,0);
  lcd.write(dig[h10][0]); delay(DELAY);
  lcd.write(dig[h10][1]);  delay(DELAY);
  lcd.write(dig[h1][0]); delay(DELAY);
  lcd.write(dig[h1][1]); delay(DELAY);
  lcd.write(LB); delay(DELAY);
  lcd.write(dig[m10][0]); delay(DELAY);
  lcd.write(dig[m10][1]); delay(DELAY);
  lcd.write(dig[m1][0]); delay(DELAY);
  lcd.write(dig[m1][1]); delay(DELAY);
//
  lcd.setCursor(pos,1); delay(DELAY);
  lcd.write(dig[h10][2]); delay(DELAY);
  lcd.write(dig[h10][3]); delay(DELAY);
  lcd.write(dig[h1][2]); delay(DELAY); 
  lcd.write(dig[h1][3]); delay(DELAY);
  lcd.write(UB); delay(DELAY);
  lcd.write(dig[m10][2]); delay(DELAY); 
  lcd.write(dig[m10][3]); delay(DELAY);
  lcd.write(dig[m1][2]); delay(DELAY);
  lcd.write(dig[m1][3]); delay(DELAY);
}  

void clockSetup(int rate) {
  // set up the LCD's number of columns and rows: 
  lcd.begin(16,2);
  lcd.createChar(0,custom_0);
  lcd.createChar(1,custom_1);
  lcd.createChar(2,custom_2);
  lcd.createChar(3,custom_3);
  lcd.createChar(4,custom_4);
  lcd.createChar(5,custom_5);
  lcd.createChar(6,custom_6);
  lcd.createChar(7,custom_7);
  lcd.begin(16, 2);  
  lcd.clear();
  lcd.setCursor(0,0); delay(DELAY); lcd.print("   OpenLCB");
  lcd.setCursor(0,1); lcd.print("   Rules OK");
  delay(3000);
  lcd.setCursor(0,0); lcd.print("Rate:    ");
  lcd.setCursor(0,1); lcd.print("         ");
  lcd.setCursor(0,1); lcd.print(rate); lcd.print(":1    ");
  showTime(time, 6);
}

  

