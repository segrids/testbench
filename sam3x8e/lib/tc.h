#ifndef TC_H_
#define TC_H_

#include "types.h"

typedef struct {
  volatile uint32_t      TC_CCR;
  volatile uint32_t      TC_CMR;
  volatile uint32_t      TC_SMMR;
  volatile uint32_t      space1[1];
  volatile uint32_t      TC_CV;
  volatile uint32_t      TC_RA;
  volatile uint32_t      TC_RB;
  volatile uint32_t      TC_RC;
  volatile uint32_t      TC_SR;
  volatile uint32_t      TC_IER;
  volatile uint32_t      TC_IDR;
  volatile uint32_t      TC_IMR;
  volatile uint32_t      space2[4];
} TcChannel;

typedef struct {
  TcChannel  TC_CHANNEL[3];
  volatile uint32_t      TC_BCR;
  volatile uint32_t      TC_BMR;
  volatile uint32_t      TC_QIER;
  volatile uint32_t      TC_QIDR;
  volatile uint32_t      TC_QIMR;
  volatile uint32_t      TC_QISR;
  volatile uint32_t      TC_FMR;
  volatile uint32_t      space1[2];
  volatile uint32_t      TC_WPMR;
} TcModule;


// To make things clear and consistent with the SAM3x datasheet:
// Distinguish between TC channels and modules, and
// use the mnemonics TC0-TC8 for channels instead of modules! 

#define TCM0 ((TcModule *)0x40080000)
#define TCM1 ((TcModule *)0x40084000)
#define TCM2 ((TcModule *)0x40088000)

#define TC0 ((TcChannel *)0x40080000)
#define TC1 ((TcChannel *)0x40080040)
#define TC2 ((TcChannel *)0x40080080)
#define TC3 ((TcChannel *)0x40084000)
#define TC4 ((TcChannel *)0x40084040)
#define TC5 ((TcChannel *)0x40084080)
#define TC6 ((TcChannel *)0x40088000)
#define TC7 ((TcChannel *)0x40088040)
#define TC8 ((TcChannel *)0x40088080)



void tc_pmc_enable(uint32_t channel);
void tc_pmc_disable(uint32_t channel);
void tc_reset(TcChannel * pTc);
uint32_t tc_get_value(TcChannel * pTc);
void tc_nvic_enable(uint32_t channel);
void tc_nvic_disable(uint32_t channel);
uint32_t tc_deassert_irq(TcChannel* pTc);
void tc_enable_interrupt(TcChannel * pTc, uint32_t number);
void tc_disable_interrupt(TcChannel * pTc, uint32_t number);
void tc_init_capture_mode(TcChannel * pTc, uint32_t c_value, uint32_t clock_select);
void tc_init_waveform_mode(TcChannel * pTc);

#endif
