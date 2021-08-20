#ifndef RSTC_H_
#define RSTC_H_

#include "types.h"

typedef struct {
  volatile uint32_t RSTC_CR;
  volatile uint32_t RSTC_SR;
  volatile uint32_t RSTC_MR;
} Rstc;

#define RTSTC ((Rstc *)0x400E1A00)


void rstc_reset(Rstc* p_rstc, uint8_t mask);

#endif
