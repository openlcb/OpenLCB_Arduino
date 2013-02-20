#include <OlcbUtil.h>
#include <stdint.h>

#include <unistd.h>

#ifdef __cplusplus
extern "C"{
#endif

static int time = 0;
// 1 tic per call as a temporary measure
long unsigned millis() {
    usleep(100);  // wait 0.1 msec
    return time++;
}

void tick(int count) {
    time += count;
    usleep(count*1000);  // wait count msec
}

int min(int a, int b) {
    return (a<b) ? a : b;
};

void pinMode(uint8_t, uint8_t) {}
int digitalRead(uint8_t) { return HIGH; }
void digitalWrite(uint8_t, uint8_t) {}


#ifdef __cplusplus
}
#endif
