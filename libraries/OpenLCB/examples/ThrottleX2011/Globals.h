#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "Throttle.h"
#include "PCD8544.h"

#define DISP_MAIN 0
#define DISP_LOCO_SELECT 1

extern unsigned short global_state;
extern Throttle *global_throttle;
extern PCD8544 global_lcd;

#endif
