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
 
#include "master.h"

#include "uart.h"
#include "usart.h"
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
	if (protocol == 'I') {
		// I2C master interface on Arduino Due is always TWI0
		twi_enable(TWI1);
		twi_set_master_mode(TWI1, config);
		master_interface.pointer = (void *)TWI1;
		master_interface.send_data = (int (*)(void *, uint8_t, uint8_t *, int)) &twi_master_send_data;
		master_interface.receive_data = (int (*)(void *, uint8_t, uint8_t *, int)) &twi_master_receive_data;
		master_interface.close = (void (*)(void *)) &twi_close;
	} else if (protocol == 'S') {
		// SPI master interface on Arduino Due is SPI0 (since only one SPI interface, slave as well)
		spi_enable(SPI0, 1); // 1=MASTER MODE
		spi_flush_receiver(SPI0);
		master_interface.pointer = (void *)SPI0;
		master_interface.send_data = (int (*)(void *, uint8_t, uint8_t *, int)) &spi_master_send_data;
		master_interface.receive_data = (int (*)(void *, uint8_t, uint8_t *, int)) &spi_master_receive_data;
		master_interface.close = (void (*)(void *)) &spi_close;
	} else if (protocol == 'H') {
		hdq_init(HDQ0, 0);
		master_interface.pointer = (void *)&hdq;
		// HDQ interface (Arduino Due Pins 14 and 15)
		master_interface.send_data = (int (*)(void *, uint8_t, uint8_t *, int)) &hdq_master_send_data;
		master_interface.receive_data = (int (*)(void *, uint8_t, uint8_t *, int)) &hdq_master_receive_data;
		master_interface.close = (void (*)(void *)) &hdq_close;
	} else if (protocol == 'V') {
		usart_configure(USART1, 24000000, 115200); // TODO: configuration depending on `config`
		master_interface.pointer = (void *)USART1;
		master_interface.send_data = (int (*)(void *, uint8_t, uint8_t *, int)) &usart_master_send_data;
		master_interface.receive_data = (int (*)(void *, uint8_t, uint8_t *, int)) &usart_master_receive_data;
		master_interface.close = (void (*)(void *)) &usart_close;
		
	} else {
		return -1; // TODO: Error code
	}
	return 0;
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

