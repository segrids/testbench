#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "types.h"

typedef struct { 
	void* pointer;
	int (*send_uint8)(void *, uint8_t);
	int (*receive_uint8)(void *, uint8_t *);
	void (*close)(void *);
} slave_t; 

int slave_init(uint8_t protocol, uint8_t slave_address, uint8_t config);
int slave_send_uint8(uint8_t byte);
int slave_send_uint16(uint16_t nibble);
int slave_send_uint32(uint32_t word);
int slave_send_data(uint8_t *data, int length);
int slave_receive_uint8(uint8_t *p_byte);
int slave_receive_uint16(uint16_t *p_nibble);
int slave_receive_uint32(uint32_t *p_word);
int slave_receive_data(uint8_t *data, int length);
void slave_close(void);

#ifdef FLASH
typedef struct { 
	void* pointer;
	int (*send_data)(void *, uint8_t, uint8_t *, int);
	int (*receive_data)(void *, uint8_t, uint8_t *, int);
	void (*close)(void *);
} master_t; 


int master_init(uint8_t protocol, uint8_t config);
int master_send_data(uint8_t slave_address, uint8_t *data, int length);
int master_receive_data(uint8_t slave_address, uint8_t *data, int length);
void master_close(void);

#endif
#endif
