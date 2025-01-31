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
 
#include "apdu.h"

#include "i2c.h"

#define SR_BUSBUSY (1 << 19)
#define SR_TXDE (1 << 17)
#define SR_RXDNE (1 << 16)
#define SR_ADRS (1 << 2)
#define SR_STO (1 << 1)

void i2c_configure(I2c* i2c) {
	i2c->CR = 0x00000009;
	i2c->SHPGR = 34;
	i2c->SLPGR = 34;
}

void i2c_write_to(I2c* i2c, uint8_t addr) {
	while (i2c->SR & SR_BUSBUSY) { }
	i2c->TAR = addr & 0x7f;
}

void i2c_read_from(I2c* i2c, uint8_t addr) {
	while (i2c->SR & SR_BUSBUSY) { }
	i2c->TAR = (addr & 0x7f) | (1 << 10);
}

uint8_t i2c_read_byte(I2c* i2c) {
	while (!(i2c->SR & SR_RXDNE)) { }
	return (uint8_t) i2c->DR;
}

void i2c_write_byte(I2c* i2c, uint8_t byte) {
	while (!(i2c->SR & SR_TXDE)) { }
	i2c->DR = (uint32_t) byte;
}

void i2c_send_stop(I2c* i2c) {
	i2c->CR |= 1 << 1;
}

void i2c_slave_set_address(I2c* i2c, uint8_t addr) {
	i2c->ADDR = addr;
}

int i2c_slave_receive(I2c* i2c, uint8_t * buffer, int size) {
	while (!(i2c->SR & SR_ADRS)) { }
	int i = 0;
	for (; i < size; i++) {
		uint32_t status;
		while (!((status = i2c->SR) & SR_RXDNE)) {
			if (status & SR_STO) {
				return i;
			}
		}
		buffer[i] = i2c->DR;
	}
	return i;
}

int i2c_slave_transmit(I2c* i2c, uint8_t * buffer, int size) {
	while (!(i2c->SR & SR_ADRS)) { }
	int i = 0;
	for (; i < size; i++) {
		uint32_t status;
		while (!((status = i2c->SR) & SR_TXDE)) {
			if (status & SR_STO) {
				return i;
			}
		}
		i2c->DR = buffer[i];
	}
	return i;
}

// Common communication interface

int i2c_slave_receive_apdu(I2c *i2c) {
	int n = i2c_slave_receive(i2c, (uint8_t *) &apdu, sizeof(apdu_t));
	if (n >= 6) {
		uint8_t *buf = (uint8_t *) &apdu;
		apdu.le = (buf[n - 1] << 8) | buf[n - 2];
		// TODO: Check if data length matches apdu.lc.
		return 0;
	} else {
		return 1;
	}
}

int i2c_slave_send_apdu_response(I2c *i2c, uint8_t *b, int n) {
	return i2c_slave_transmit(i2c, b, n) == n;
}
