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
 
#include "uart.h"
#include "pio.h"


/* uart_flush() 
 *
 * Flush the UART interface. 
 */
int uart_flush(Uart* p_uart) {
	p_uart->UART_CR = 0x15c;
	return 0;
}

/* uart_close()
 *
 * No need to close an interface so far.
 */
void uart_close(Uart* p_uart) {
	return;
}


/* uart_configure()
 *
 * Enable UARTR and set the baudrate. 
 */
void uart_configure(Uart* p_uart, uint32_t mck, uint32_t baud_rate) {
	pio_select_peripheral_A(PIOA, 0x3 << 8);

	uint16_t cd = mck / (baud_rate * 16);
	p_uart->UART_BRGR = cd;

	p_uart->UART_MR = 0x800;
	p_uart->UART_CR = 0x50;
}


/* uart_receive_uint8()
 *
 * Receive one character. 
 */
int uart_receive_uint8(void* pointer, uint8_t *p) {
	Uart *p_uart = (Uart *)pointer;
	while ((p_uart->UART_SR & 0x01) == 0) { }
	*p = (uint8_t)p_uart->UART_RHR;
	return 0;
}

/* uart_send_uint8()
 *
 * Send one character. 
 */
int uart_send_uint8(void* pointer, uint8_t byte) {
	Uart *p_uart = (Uart *)pointer;
	while ((p_uart->UART_SR & 0x02) == 0) { }
	return 0;
}


/* uart_pdc_transfer()
 *
 * Send data from memory to UART by peripheral DMA.
 */
void uart_pdc_transfer(Uart* p_uart, uint8_t * p, uint32_t count) {
	p_uart->UART_TPR = (uint32_t) p;
	p_uart->UART_TCR = count;
	p_uart->UART_PTCR = 0x100;
	p_uart->UART_IER = 0x2;
}


