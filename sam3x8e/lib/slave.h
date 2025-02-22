#ifndef SLAVE_H_
#define SLAVE_H_

#include "types.h"

typedef struct { 
	void* pointer;
	void (*send_uint8)(void *, uint8_t);
	void (*receive_uint8)(void *, uint8_t *);
	void (*close)(void *);
} slave_t; 

int slave_init(uint8_t protocol, uint8_t slave_address, uint8_t config);
void slave_send_uint8(uint8_t byte);
void slave_send_uint16(uint16_t nibble);
void slave_send_uint32(uint32_t word);
void slave_send_data(uint8_t *data, int length);
void slave_receive_uint8(uint8_t *p_byte);
void slave_receive_uint16(uint16_t *p_nibble);
void slave_receive_uint32(uint32_t *p_word);
void slave_receive_data(uint8_t *data, int length);
void slave_close(void);

#endif
