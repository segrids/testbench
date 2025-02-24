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
  volatile uint32_t space[53];
  volatile uint32_t USART_RPR;   // See PDC section of Datasheet
  volatile uint32_t USART_RCR;
  volatile uint32_t USART_TPR;
  volatile uint32_t USART_TCR;
  volatile uint32_t USART_RNPR;
  volatile uint32_t USART_RNCR;
  volatile uint32_t USART_TNPR;
  volatile uint32_t USART_TNCR;
  volatile uint32_t USART_PTCR;
  volatile uint32_t USART_PTSR;
} Usart;

#define USART1       ((Usart   *)0x4009C000)

void usart_configure(Usart * p_uart, uint32_t mck, uint32_t baud_rate);
void usart_send_uint8(Usart* pointer, uint8_t byte);
void usart_receive_uint8(Usart* pointer, uint8_t *p);
void usart_flush(Usart* p_usart);
void usart_close(Usart* p_usart);
void usart_set_rts(Usart* p_usart, int value);
int usart_master_send_data(Usart *p_usart, uint8_t dummy_address, uint8_t *buffer, uint32_t buffer_len);
int usart_master_receive_data(Usart *p_usart, uint8_t dummy_address, uint8_t *buffer, uint32_t buffer_len);
void usart_pdc_sendreceive(Usart* p_usart, uint8_t dummy_address, 
                uint8_t *data, int send_len, uint8_t *buffer, int res_len);
#endif
