#ifndef USART_H_
#define USART_H_

#include "types.h"

typedef struct {
  volatile uint32_t USART_CR;
  volatile uint32_t USART_MR;
  volatile uint32_t USART_IER;
  volatile uint32_t USART_IDR;  // Interrupt Disable
  volatile uint32_t USART_IMR;  // Interrupt Mask
  volatile uint32_t USART_CSR;  // Channel Status
  volatile uint32_t USART_RHR;  // Receiver Holding
  volatile uint32_t USART_THR;  // Transmitter Holding
  volatile uint32_t USART_BRGR; // Baud Rate Generation
  volatile uint32_t USART_RTOR; // Receiver Timeout
  volatile uint32_t USART_TTGR; // Transmitter Timeguard
} Usart;

#define USART1       ((Usart   *)0x4009C000)

void usart_configure(Usart * p_uart, uint32_t mck, uint32_t baud_rate);
void usart_send_uint8(Usart* pointer, uint8_t byte);
void usart_receive_uint8(Usart* pointer, uint8_t *p);
void usart_flush(Usart* p_usart);
void usart_close(Usart* p_usart);
void usart_set_rts(Usart* p_usart, int value);
int usart_master_send_data(Usart *p_usart, uint8_t slave_address, uint8_t *buffer, uint32_t buffer_len);
int usart_master_receive_data(Usart *p_usart, uint8_t slave_address, uint8_t *buffer, uint32_t buffer_len);
#endif
