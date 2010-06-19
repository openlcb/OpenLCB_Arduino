// Arduino stand-in
#include <stdint.h>

// 1 tic per call as a temporary measure
long millis() {
    static int i;
    return ++i;
}

int min(int a, int b) {
    return (a<b) ? a : b;
};