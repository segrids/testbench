#ifndef MASTER_H_
#define MASTER_H_

#include "types.h"

typedef struct { 
	uint8_t receive_buffer[256];
	void* pointer;
	int (*send_data)(void *, uint8_t, uint8_t *, int);
	int (*receive_data)(void *, uint8_t, uint8_t *, int);
	void (*sendreceive)(void *, uint8_t, uint8_t *, int, uint8_t *, int);
	void (*close)(void *);
	void (*flush)(void *);
} master_t; 

extern master_t master_interface;

/*
 * master_send_data() gets a pointer to the send buffer as argument whereas
 * master_receive_data() always writes to the fixed receive_buffer.
 */
int master_init(uint8_t protocol, uint8_t config);
int master_send_data(uint8_t slave_address, uint8_t *data, int length);
int master_receive_data(uint8_t slave_address, int length);
void master_sendreceive(uint8_t slave_address, uint8_t *data, int length, int res_len);
void master_close(void);
void master_flush(void);
void master_clear_buffer(void);

#endif
