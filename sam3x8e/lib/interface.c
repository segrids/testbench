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
 
#include "interface.h"
#include "uart.h"
#include "twi.h"
#include "spi.h"
#include "hdq.h"

/* We use the Arduino Due typically as a slave with the PC as the master.
 * It might furtheremore act as a master (e.g. when used as an adapter)
 * with another device as slave.
 *
 * We want to change the master and slave interfaces flexibly. Therefore,
 * we implement master and slave interfaces as an abstraction layer between
 * peripheral hardware (UART, I2C, SPI, HDQ) and the APDU handler.
 */

master_t master_interface;
slave_t slave_interface;

/* slave_init()
 *
 * Intialize slave_interface
 *
 * Called once at the beginning of `main()`. Protocol selected by jumper config.
 *
 * args: protocol in {'U': UART, 'S': SPI, 'I': I2C}
 *       slave_address: only relevant for protocol 'I' so far
 *       config: protocol specific 32-bit encoding of interface configuration 
 */
int slave_init(uint8_t protocol, uint8_t slave_address, uint8_t config){
	if (protocol == 'U') {
		// UART
		uart_configure(UART, 24000000, 115200);
		slave_interface.pointer = (void *)UART;
		slave_interface.send_uint8 = (int (*)(void *, uint8_t)) &uart_send_uint8;
		slave_interface.receive_uint8 = (int (*)(void *, uint8_t *)) &uart_receive_uint8;
		slave_interface.close = (void (*)(void *)) &uart_close;
	} else if (protocol == 'I') {
		// I2C slave interface on Arduino Due is always TWI0
		twi_enable(TWI0);
		twi_set_slave_mode(TWI0, slave_address);
		slave_interface.pointer = (void *)TWI0;
		slave_interface.send_uint8 = (int (*)(void *, uint8_t)) &twi_send_uint8;
		slave_interface.receive_uint8 = (int (*)(void *, uint8_t *)) &twi_receive_uint8;
		slave_interface.close = (void (*)(void *)) &twi_close;
	} else if (protocol == 'S') {
		// SPI slave interface on Arduino Due is SPI0 (since only one SPI interface, master as well)
		spi_enable(SPI0, 0); // 0=SLAVE MODE
		slave_interface.pointer = (void *)SPI0;
		slave_interface.send_uint8 = (int (*)(void *, uint8_t)) &spi_send_uint8;
		slave_interface.receive_uint8 = (int (*)(void *, uint8_t *)) &spi_receive_uint8;
		slave_interface.close = (void (*)(void *)) &spi_close;
	} else if (protocol == 'H') {
		hdq_init(HDQ0, 0);
		slave_interface.pointer = (void *)&hdq; // equivalent to malloc(sizeof(Hdq)) ???
		slave_interface.send_uint8 = (int (*)(void *, uint8_t)) &hdq_send_uint8;
		slave_interface.receive_uint8 = (int (*)(void *, uint8_t *)) &hdq_receive_uint8;
		slave_interface.close = (void (*)(void *)) &hdq_close;
	} else {
		return -1; // TODO: Error code
	}
	return 0;
}

/* slave_send_uint8()
 *
 * Abstraction of peripheral interface specific send method.
 *
 */
int slave_send_uint8(uint8_t byte){
	return slave_interface.send_uint8(slave_interface.pointer, byte);
}

/* slave_send_uint16()
 *
 * Send uint_16.
 */
int slave_send_uint16(uint16_t nibble){
	int ret;
	ret = slave_send_uint8((uint8_t)nibble);
	ret |= slave_send_uint8((uint8_t)(nibble >> 8));
	return ret;
}

/* slave_send_uint32()
 *
 * Send uint_32.
 */
int slave_send_uint32(uint32_t word){
	int ret;
	ret = slave_send_uint16((uint16_t)word);
	ret |= slave_send_uint16((uint16_t)(word >> 16));
	return ret;
}

/* slave_send_data()
 *
 * Send buffer of given length.
 */
int slave_send_data(uint8_t *data, int length){
	int ret = 0;
	for (int i=0; i<length; i++){
		ret |= slave_send_uint8(data[i]);
	}
	return ret;
}

/* slave_receive_uint8()
 *
 * Abstraction of peripheral interface specific receive method.
 *
 */
int slave_receive_uint8(uint8_t *p_byte){
	return slave_interface.receive_uint8(slave_interface.pointer, p_byte);
}

/* slave_receive_uint16()
 *
 * Reveive uint16_t.
 */
int slave_receive_uint16(uint16_t *p_nibble){
	int ret;
	ret = slave_receive_uint8((uint8_t *)p_nibble);
	ret |= slave_receive_uint8(((uint8_t *)p_nibble) + 1);
	return ret;
}
/*  slave_receive_uint32()
 *
 * Receive four characters encoding a uint32_t, LSB first.
 * Important feature: buffer the uint32_t in a local variable and write word wise 
 * to the target location. To be used for flash writes, where only word writes are allowed.
 */
int slave_receive_uint32(uint32_t *p_word){
	int ret;
	uint32_t word;
	ret = slave_receive_uint16((uint16_t *)(&word));
	ret |= slave_receive_uint16((uint16_t *)(&word) + 1);
	*p_word = word;
	return ret;
}

/* slave_send_data()
 *
 * Receive data of given length.
 */
int slave_receive_data(uint8_t *data, int length){
	int ret = 0;
	for (int i=0; i<length; i++){
		ret |= slave_receive_uint8(data + i);
	}
	return ret;
}

/* slave_close() */
void slave_close(void){
	slave_interface.close(slave_interface.pointer);
}



/* master_init()
 *
 * Intialize master_interface.
 *
 * Called if requested by apdu.cla == 'A', apdu.ins == 'O'.
 * apdu.data encodes the args.
 *
 * args: protocol in {'U': UART, 'S': SPI, 'I': I2C, 'H': HDQ}
 *       config: protocol specific 8-bit encoding of interface configuration 
 */
int master_init(uint8_t protocol, uint8_t config){
	int ret;
	if (protocol == 'I') {
		// I2C master interface on Arduino Due is always TWI0
		twi_enable(TWI1);
		ret = twi_set_master_mode(TWI1, config);
		master_interface.pointer = (void *)TWI1;
		master_interface.send_data = (int (*)(void *, uint8_t, uint8_t *, int)) &twi_master_send_data;
		master_interface.receive_data = (int (*)(void *, uint8_t, uint8_t *, int)) &twi_master_receive_data;
		master_interface.close = (void (*)(void *)) &twi_close;
	} else if (protocol == 'S') {
		// SPI master interface on Arduino Due is SPI0 (since only one SPI interface, slave as well)
		spi_enable(SPI0, 1); // 1=MASTER MODE
		spi_flush_receiver(SPI0);
		ret = 0;
		master_interface.pointer = (void *)SPI0;
		master_interface.send_data = (int (*)(void *, uint8_t, uint8_t *, int)) &spi_master_send_data;
		master_interface.receive_data = (int (*)(void *, uint8_t, uint8_t *, int)) &spi_master_receive_data;
		master_interface.close = (void (*)(void *)) &spi_close;
	} else if (protocol == 'H') {
		ret = 0;
		hdq_init(HDQ0, 0);
		master_interface.pointer = (void *)&hdq; // equivalent to malloc(sizeof(Hdq)) ???
		// HDQ interface (Arduino Due Pins 14 and 15)
		master_interface.send_data = (int (*)(void *, uint8_t, uint8_t *, int)) &hdq_master_send_data;
		master_interface.receive_data = (int (*)(void *, uint8_t, uint8_t *, int)) &hdq_master_receive_data;
		master_interface.close = (void (*)(void *)) &hdq_close;
		
	} else {
		return -1; // TODO: Error code
	}
	return ret;
}

/* master_send_data()
 *
 * Abstraction of periphal interface specific send data function.
 */
int master_send_data(uint8_t slave_address, uint8_t *buffer, int len){
	return master_interface.send_data(master_interface.pointer, slave_address, buffer, len);
}

/* master_receive_data()
 *
 * Abstraction of periphal interface specific receive data function.
 */
int master_receive_data(uint8_t slave_address, uint8_t *buffer, int len){
	return master_interface.receive_data(master_interface.pointer, slave_address, buffer, len);
}

/* master_close() */
void master_close(void){
	master_interface.close(master_interface.pointer);
}

