#ifndef UART_H_
#define UART_H_

#include "types.h"

typedef struct {
  volatile uint32_t UART_CR;
  volatile uint32_t UART_MR;
  volatile uint32_t UART_IER;
  volatile uint32_t UART_IDR;
  volatile uint32_t UART_IMR;
  volatile uint32_t UART_SR;
  volatile uint32_t UART_RHR;
  volatile uint32_t UART_THR;
  volatile uint32_t UART_BRGR;
  volatile uint32_t space[55];
  volatile uint32_t UART_RPR;
  volatile uint32_t UART_RCR;
  volatile uint32_t UART_TPR;
  volatile uint32_t UART_TCR;
  volatile uint32_t UART_RNPR;
  volatile uint32_t UART_RNCR;
  volatile uint32_t UART_TNPR;
  volatile uint32_t UART_TNCR;
  volatile uint32_t UART_PTCR;
  volatile uint32_t UART_PTSR;
} Uart;

#define UART       ((Uart   *)0x400E0800)
#define PDC_UART   ((Pdc    *)0x400E0900)

void uart_configure(Uart * p_uart, uint32_t mck, uint32_t baud_rate);
void uart_pdc_transfer(Uart* p_uart, uint8_t * p, uint32_t count);
int uart_send_uint8(Uart* pointer, uint8_t byte);
int uart_receive_uint8(Uart* pointer, uint8_t *p);
int uart_flush(Uart* p_uart);
void uart_close(Uart* p_uart);
#endif
