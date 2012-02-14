// Arduino stand-in
#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

// 1 tic per call as a temporary measure
long millis() {
    static int i;
    return ++i;
}

int min(int a, int b) {
    return (a<b) ? a : b;
};

#ifdef __cplusplus
}
#endif
