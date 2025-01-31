/* MIT License

Copyright (c) 2021 by SEGRIDS GmbH <www.segrids.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. 

Author: Frank Schuhmacher <frank.schuhmacher@segrids.com>
*/
 
#ifndef I2C_H_
#define I2C_H_

#include "types.h"

typedef struct {
	volatile uint32_t CR;
	volatile uint32_t IER;
	volatile uint32_t ADDR;
	volatile uint32_t SR;
	volatile uint32_t SHPGR;
	volatile uint32_t SLPGR;
	volatile uint32_t DR;
	volatile uint32_t TAR;
	volatile uint32_t ADDMR;
	volatile const uint32_t ADDSR;
	volatile uint32_t TOUT;
} I2c;

#define I2C0 ((I2c *) 0x40048000)
#define I2C1 ((I2c *) 0x40049000)

void i2c_configure(I2c* i2c);
void i2c_write_to(I2c* i2c, uint8_t addr);
void i2c_read_from(I2c* i2c, uint8_t addr);
uint8_t i2c_read_byte(I2c* i2c);
void i2c_write_byte(I2c* i2c, uint8_t byte);
void i2c_send_stop(I2c* i2c);
void i2c_slave_set_address(I2c* i2c, uint8_t addr);
int i2c_slave_receive(I2c* i2c, uint8_t * buffer, int size);
int i2c_slave_transmit(I2c* i2c, uint8_t * buffer, int size);
int i2c_slave_receive_apdu(I2c *i2c);
int i2c_slave_send_apdu_response(I2c *i2c, uint8_t *b, int n);

#endif
