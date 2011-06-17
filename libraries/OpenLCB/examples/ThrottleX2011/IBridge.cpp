#include "IBridge.h"


int IBridge_Column_Pin0 = 7;
int IBridge_Column_Pin1 = 6;
int IBridge_Column_Pin2 = 5;
int IBridge_Column_Pin3 = 4;

int IBridge_Row_Pin0 = 3;
int IBridge_Row_Pin1 = 2;
int IBridge_Row_Pin2 = 58;
int IBridge_Row_Pin3 = 59;

void IBridge_init()
{
	IBridge_GPIO_Config();
}

void IBridge_GPIO_Config()
{
  pinMode(IBridge_Column_Pin0, OUTPUT);
  pinMode(IBridge_Column_Pin1, OUTPUT);
  pinMode(IBridge_Column_Pin2, OUTPUT);
  pinMode(IBridge_Column_Pin3, OUTPUT);

  pinMode(IBridge_Row_Pin0, INPUT);
  pinMode(IBridge_Row_Pin1, INPUT);
  pinMode(IBridge_Row_Pin2, INPUT);
  pinMode(IBridge_Row_Pin3, INPUT);
}

unsigned char IBridge_Read_Key()
{
  //unsigned char i = 10;
  boolean a,b,c,d;
  //Column 0 scan

  digitalWrite(IBridge_Column_Pin1, LOW);
  digitalWrite(IBridge_Column_Pin2, LOW);
  digitalWrite(IBridge_Column_Pin3, LOW);
  digitalWrite(IBridge_Column_Pin0, HIGH);
 
  //i=10;
  //while(i--);
  delay(1);

  a = digitalRead(IBridge_Row_Pin0);
  b = digitalRead(IBridge_Row_Pin1);
  c = digitalRead(IBridge_Row_Pin2);
  d = digitalRead(IBridge_Row_Pin3);
  
  if(a && !b && !c && !d)
    return (1);

  if(!a &&  b && !c && !d)
    return (2);

  if(!a && !b &&  c && !d)
    return (3);

  if(!a && !b && !c &&  d)
    return (4);

  //Column 2 Scan

  digitalWrite(IBridge_Column_Pin0, LOW);
  digitalWrite(IBridge_Column_Pin1, HIGH);
  digitalWrite(IBridge_Column_Pin2, LOW);
  digitalWrite(IBridge_Column_Pin3, LOW);

  //i=10;
  //while(i--);
  delay(1);
  
  a = digitalRead(IBridge_Row_Pin0);
  b = digitalRead(IBridge_Row_Pin1);
  c = digitalRead(IBridge_Row_Pin2);
  d = digitalRead(IBridge_Row_Pin3);

  if(a && !b && !c && !d)
    return (5);

  if(!a &&  b && !c && !d)
    return (6);

  if(!a && !b &&  c && !d)
    return (7);

  if(!a && !b && !c &&  d)
    return (8);

  //Column 3 Scan

  digitalWrite(IBridge_Column_Pin0, LOW);
  digitalWrite(IBridge_Column_Pin1, LOW);
  digitalWrite(IBridge_Column_Pin2, HIGH);
  digitalWrite(IBridge_Column_Pin3, LOW);

  //i=10;
  //while(i--);
  delay(1);

  a = digitalRead(IBridge_Row_Pin0);
  b = digitalRead(IBridge_Row_Pin1);
  c = digitalRead(IBridge_Row_Pin2);
  d = digitalRead(IBridge_Row_Pin3);

  if(a && !b && !c && !d)
    return (9);

  if(!a &&  b && !c && !d)
    return (10);

  if(!a && !b &&  c && !d)
    return (11);

  if(!a && !b && !c &&  d)
    return (12);

  //Column 4 Scan

  digitalWrite(IBridge_Column_Pin0, LOW);
  digitalWrite(IBridge_Column_Pin1, LOW);
  digitalWrite(IBridge_Column_Pin2, LOW);
  digitalWrite(IBridge_Column_Pin3, HIGH);

  delay(1);
  
  a = digitalRead(IBridge_Row_Pin0);
  b = digitalRead(IBridge_Row_Pin1);
  c = digitalRead(IBridge_Row_Pin2);
  d = digitalRead(IBridge_Row_Pin3);

  if(a && !b && !c && !d)
    return (13);

  if(!a &&  b && !c && !d)
    return (14);

  if(!a && !b &&  c && !d)
    return (15);

  if(!a && !b && !c &&  d)
    return (16);

  return(0);

}
