#include "types.h"


typedef struct
{
	volatile uint32_t CPUID;
	volatile uint32_t ICSR;
	volatile uint32_t VTOR;
	volatile uint32_t AIRCR;
	volatile uint32_t SCR;
	volatile uint32_t CCR;
	volatile uint32_t SHPR[3];
	volatile uint32_t SHCRS;
	volatile uint32_t CFSR;
	volatile uint32_t HFSR;
	volatile uint32_t DFSR;
	volatile uint32_t MMFAR;
	volatile uint32_t BFAR;
	volatile uint32_t AFSR;
	volatile uint32_t PFR[2];
	volatile uint32_t DFR;
	volatile uint32_t ADR;
	volatile uint32_t MMFR[4];
	volatile uint32_t ISAR[5];
	volatile uint32_t space0[5];
	volatile uint32_t CPACR;
} Scb;

#define SCB ((Scb *)0xE000ED00)


void scb_enable_handlers(Scb* p_scb);
void scb_move_vectors(Scb* p_scb, uint32_t address);
void scb_reset_vectors(Scb* p_scb);
void scb_replace_vector(Scb* p_scb, uint32_t exception_number, void * fct);

