#ifndef WDT_H_
#define WDT_H_

#include "types.h"
typedef struct {
  volatile uint32_t WDT_CR; /**< \brief (Wdt Offset: 0x00) Control Register */
  volatile uint32_t WDT_MR; /**< \brief (Wdt Offset: 0x04) Mode Register */
  volatile uint32_t WDT_SR; /**< \brief (Wdt Offset: 0x08) Status Register */
} Wdt;

#define WDT        ((Wdt    *)0x400E1A50)

void wdt_enable(Wdt* p_wdt, int status);
void wdt_reset(Wdt* p_wdt);

#endif
