#ifndef PMC_H_
#define PMC_H_

#include "types.h"

typedef struct {
  volatile uint32_t PMC_SCER;
  volatile uint32_t PMC_SCDR;
  volatile uint32_t PMC_SCSR;
  volatile uint32_t Reserved1[1];
  volatile uint32_t PMC_PCER0;
  volatile uint32_t PMC_PCDR0;
  volatile uint32_t PMC_PCSR0;
  volatile uint32_t CKGR_UCKR;
  volatile uint32_t CKGR_MOR;
  volatile uint32_t CKGR_MCFR;
  volatile uint32_t CKGR_PLLAR;
  volatile uint32_t Reserved2[1];
  volatile uint32_t PMC_MCKR;
  volatile uint32_t Reserved3[1];
  volatile uint32_t PMC_USB;
  volatile uint32_t Reserved4[1];
  volatile uint32_t PMC_PCK[3];
  volatile uint32_t Reserved5[5];
  volatile uint32_t PMC_IER;
  volatile uint32_t PMC_IDR;
  volatile uint32_t PMC_SR;
  volatile uint32_t PMC_IMR;
  volatile uint32_t PMC_FSMR;
  volatile uint32_t PMC_FSPR;
  volatile uint32_t PMC_FOCR;
  volatile uint32_t Reserved6[26];
  volatile uint32_t PMC_WPMR;
  volatile uint32_t PMC_WPSR;
  volatile uint32_t Reserved7[5];
  volatile uint32_t PMC_PCER1;
  volatile uint32_t PMC_PCDR1;
  volatile uint32_t PMC_PCSR1;
  volatile uint32_t PMC_PCR;
} Pmc;


#define PMC        ((Pmc    *)0x400E0600)

void pmc_24MHz(Pmc* p_pmc );
void pmc_enable_peripheral_clock(Pmc* p_pmc, int pid);
void pmc_disable_peripheral_clock(Pmc* p_pmc,  int pid);

#endif
