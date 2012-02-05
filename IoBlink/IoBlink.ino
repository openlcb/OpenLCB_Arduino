#include <ButtonLED.h>
#define NPINS 50
ButtonLed b[NPINS];

void setup() {
  for(int i=0; i<NPINS; i++) {
    b[i].setPinSense(i, LOW);
    b[i].pattern = 0x0F0F0F0FL;
  }
}
int p;
void loop() {
  if(p>=NPINS) p=0;
  b[p++].process();
}

