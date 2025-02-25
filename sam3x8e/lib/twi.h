#ifndef TWI_H_
#define TWI_H_

#include "types.h"

typedef struct {
  volatile uint32_t TWI_CR;
  volatile uint32_t TWI_MMR;
  volatile uint32_t TWI_SMR;
  volatile uint32_t TWI_IADR;
  volatile uint32_t TWI_CWGR;
  volatile uint32_t space[3];   
  volatile uint32_t TWI_SR;
  volatile uint32_t TWI_IER;
  volatile uint32_t TWI_IDR;
  volatile uint32_t TWI_IMR;
  volatile uint32_t TWI_RHR;
  volatile uint32_t TWI_THR;
} Twi;

#define TWI0       ((Twi   *)0x4008C000)
#define TWI1       ((Twi   *)0x40090000)

void twi_enable(Twi* p_twi);
void twi_reset(Twi* p_twi);
void twi_set_slave_mode(Twi* p_twi, uint8_t slave_address);
void twi_receive_uint8(void* pointer, uint8_t* byte);
void twi_send_uint8(void* pointer, uint8_t byte);
void twi_close(void* pointer);
void twi_flush(void* pointer);
int twi_set_master_mode(Twi* p_twi, uint8_t config);
int twi_master_send_data(Twi* p_twi, uint8_t slave_address, uint8_t* data, int length);
int twi_master_receive_data(Twi* p_twi, uint8_t slave_address, uint8_t* data, int length);


#endif
