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
  volatile uint32_t UART_RPR;      // PDC Receive Pointer
  volatile uint32_t UART_RCR;      // PDC Receive Counter
  volatile uint32_t UART_TPR;      // PDC Transmit Pointer
  volatile uint32_t UART_TCR;      // PDC Transmit Counter
  volatile uint32_t UART_RNPR;     // PDC Next Receive Pointer
  volatile uint32_t UART_RNCR;     // PDC Next Receive Counter
  volatile uint32_t UART_TNPR;     // PDC Next Transmit Pointer
  volatile uint32_t UART_TNCR;     // PDC Next Transmit Counter
  volatile uint32_t UART_PTCR;     // PDC Control (Enable/Disable RX/TX) 
  volatile uint32_t UART_PTSR;     // PDC Status
} Uart;

#define UART       ((Uart   *)0x400E0800)
#define PDC_UART   ((Pdc    *)0x400E0900)

void uart_configure(Uart * p_uart, uint32_t mck, uint32_t baud_rate);
void uart_pdc_send(Uart* p_uart, uint8_t * p, uint32_t count);
void uart_send_uint8(Uart* pointer, uint8_t byte);
void uart_receive_uint8(Uart* pointer, uint8_t *p);
void uart_flush(Uart* p_uart);
void uart_close(Uart* p_uart);
#endif
