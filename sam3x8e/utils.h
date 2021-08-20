#ifndef UTILS_H_
#define UTILS_H_

#include "types.h"

void utils_set_led(void);
void utils_clear_led(void);
void utils_toggle_led(void);
void utils_set_red(void);
void utils_clear_red(void);
void utils_toggle_red(void);
void utils_blink(int times, int delay);
void utils_error(int code);

#endif
