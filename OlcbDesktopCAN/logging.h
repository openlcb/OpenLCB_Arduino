// define some rudimentary logging
// intended for inclusion in .cpp files for Arduino

#ifndef lognl
#include <stdio.h>
#define logstr(...) printf(__VA_ARGS__)
#define loghex(...) printf("%x",__VA_ARGS__)
#define lognl() printf("\n")
#endif  // logln
