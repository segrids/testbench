#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "types.h"

typedef struct
{
  volatile uint32_t CTRL;
  volatile uint32_t LOAD;
  volatile uint32_t VAL;
  volatile uint32_t  CALIB;
} Systick;

#define SYSTICK       ((Systick   *)0xE000E010)

int systick_enable(Systick* p_systick, uint32_t period, uint32_t enable_interrupt, uint32_t div8);
void systick_disable(Systick* p_systick);
uint32_t systick_get_value(Systick* p_systick);
void systick_clear_value(Systick* p_systick);

#endif
