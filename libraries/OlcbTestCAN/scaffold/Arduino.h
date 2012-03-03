// Arduino stand-in
#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

// 1 tic per call as a temporary measure
long millis();

#define OUTPUT 0x1
#define INPUT 0x0
#define LOW 0x0
#define HIGH 0x1

void pinMode(uint8_t, uint8_t);
int digitalRead(uint8_t);
void digitalWrite(uint8_t, uint8_t);

#ifdef __cplusplus
}
#endif

