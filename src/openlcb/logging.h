// define some rudimentary logging
// intended for inclusion in .cpp files for Arduino

#ifndef logln
#include <stdio.h>
#define log(...) printf(__VA_ARGS__)
#define loghex(...) printf("%h",__VA_ARGS__)
#define logln() printf("\n")
#endif  // logln
