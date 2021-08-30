#ifndef MASTER_H_
#define MASTER_H_

#include "types.h"

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
