

#ifndef NVIC_H_
#define NVIC_H_

#include "types.h"

typedef struct
{
	volatile uint32_t ISER[2];
	volatile uint32_t space0[30];
	volatile uint32_t ICER[2];
	volatile uint32_t space1[30];
	volatile uint32_t ISPR[2];
	volatile uint32_t space2[30];
	volatile uint32_t ICPR[2];
	volatile uint32_t space3[30];
	volatile uint32_t IABR[2];
	volatile uint32_t space4[62];
	volatile uint8_t IP[32];
	volatile uint32_t space5[696];
	volatile uint32_t STIR;
} Nvic;

#define NVIC ((Nvic *)0xE000E100)

void nvic_enable(Nvic* p_nvic, int pid);
void nvic_disable(Nvic* p_nvic, int pid);
void nvic_clear_pending_29(void);
void nvic_set_pending(Nvic* p_nvic, int pid);
void nvic_clear_pending(Nvic* p_nvic, int pid);
int nvic_is_active(Nvic* p_nvic, int pid);
void nvic_set_priority(Nvic* p_nvic, int pid, int prio);

#endif
