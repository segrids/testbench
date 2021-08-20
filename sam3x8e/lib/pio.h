#ifndef PIO_H_
#define PIO_H_

#include "types.h"

typedef struct {
  volatile uint32_t PIO_PER;
  volatile uint32_t PIO_PDR;
  volatile uint32_t PIO_PSR;
  volatile uint32_t space1[1];
  volatile uint32_t PIO_OER;
  volatile uint32_t PIO_ODR;
  volatile uint32_t PIO_OSR;
  volatile uint32_t space2[1];
  volatile uint32_t PIO_IFER;
  volatile uint32_t PIO_IFDR;
  volatile uint32_t PIO_IFSR;
  volatile uint32_t space3[1];
  volatile uint32_t PIO_SODR;
  volatile uint32_t PIO_CODR;
  volatile uint32_t PIO_ODSR;
  volatile uint32_t PIO_PDSR;
  volatile uint32_t PIO_IER; 
  volatile uint32_t PIO_IDR;
  volatile uint32_t PIO_IMR;
  volatile uint32_t PIO_ISR;
  volatile uint32_t PIO_MDER;
  volatile uint32_t PIO_MDDR;
  volatile uint32_t PIO_MDSR;
  volatile uint32_t space4[1];
  volatile uint32_t PIO_PUDR;
  volatile uint32_t PIO_PUER;
  volatile uint32_t PIO_PUSR;
  volatile uint32_t space5[1];
  volatile uint32_t PIO_ABSR;
  volatile uint32_t space6[3];
  volatile uint32_t PIO_SCIFSR;
  volatile uint32_t PIO_DIFSR;
  volatile uint32_t PIO_IFDGSR;
  volatile uint32_t PIO_SCDR;
  volatile uint32_t space7[4];
  volatile uint32_t PIO_OWER;
  volatile uint32_t PIO_OWDR;
  volatile uint32_t PIO_OWSR;
  volatile uint32_t space8[1];
  volatile uint32_t PIO_AIMER;
  volatile uint32_t PIO_AIMDR;
  volatile uint32_t PIO_AIMMR;
  volatile uint32_t space9[1];
  volatile uint32_t PIO_ESR;
  volatile uint32_t PIO_LSR;
  volatile uint32_t PIO_ELSR;
  volatile uint32_t space10[1];
  volatile uint32_t PIO_FELLSR;
  volatile uint32_t PIO_REHLSR;
  volatile uint32_t PIO_FRLHSR;
  volatile uint32_t space11[1];
  volatile uint32_t PIO_LOCKSR;
  volatile uint32_t PIO_WPMR;
  volatile uint32_t PIO_WPSR;
} Pio;

#define PIOA       ((Pio    *)0x400E0E00)
#define PIOB       ((Pio    *)0x400E1000)
#define PIOC       ((Pio    *)0x400E1200)
#define PIOD       ((Pio    *)0x400E1400)


/* function prototypes */
int pio_get_pid(Pio* p_pio);
void pio_select_input_pins(Pio* p_pio, int pinmask);
void pio_select_output_pins(Pio* p_pio, int pinmask);
void pio_select_multidrive_pins(Pio* p_pio, int pinmask);
void pio_select_peripheral_A(Pio* p_pio, int pinmask);
void pio_select_peripheral_B(Pio* p_pio, int pinmask);
void pio_set_output_pins(Pio* p_pio, int pinmask);
void pio_clear_output_pins(Pio* p_pio, int pinmask);
void pio_toggle_output_pins(Pio* p_pio, int pinmask);
void pio_pullup_enable(Pio* p_pio, int pinmask);
uint32_t pio_read_input_pins(Pio* p_pio);

#endif
