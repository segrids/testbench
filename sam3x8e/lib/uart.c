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
void uart_flush(Uart* p_uart) {
	p_uart->UART_CR = 0x15c;
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
void uart_receive_uint8(Uart* p_uart, uint8_t *p) {
	while ((p_uart->UART_SR & 0x01) == 0) { }
	*p = (uint8_t)p_uart->UART_RHR;
}

/* uart_send_uint8()
 *
 * Send one character. 
 */
void uart_send_uint8(Uart* p_uart, uint8_t byte) {
	while ((p_uart->UART_SR & 0x02) == 0) { }
	p_uart->UART_THR = byte;
}


/* uart_pdc_transfer()
 *
 * Send data from memory to UART by peripheral DMA.
 */
void uart_pdc_send(Uart* p_uart, uint8_t * p, uint32_t count) {
	p_uart->UART_TPR = (uint32_t) p; // transmit pointer
	p_uart->UART_TCR = count;        // transmit counter
	p_uart->UART_PTCR = 0x100;       // transmit enable
	/* "The TXRDY bit triggers the PDC channel data transfer of the transmitter.
	    This results in a write of data in UART_THR."   */
	p_uart->UART_IER = 0x2;          // set TXRDY
}

/* uart_pdc_receive_enable()
 *
 * Send data from UART to memory by peripheral DMA.
 */
void uart_pdc_receive_enable(Uart* p_uart, uint8_t * p, uint32_t count) {
	p_uart->UART_RPR = (uint32_t) p;  // receive pointer
	p_uart->UART_RCR = count;         // receive counter
	p_uart->UART_PTCR = 0x1;          // receive enable
	/* "The RXRDY bit triggers the PDC channel data transfer of the receiver. 
        This results in a read of the data in UART_RHR." */
	p_uart->UART_IER = 0x1;           // set RXRDY
}

/* uart_pdc_receive_finish()
 *
 * Wait until the PDC receive is complete.
 */
void uart_pdc_receive_finish(Uart* p_uart){
	while ( (p_uart->UART_SR & (1<<3)) == 0 ){} // poll ENDRX flag
}

void uart_pdc_sendreceive(Uart* p_uart, uint8_t *data, int send_len, 
                                        uint8_t *buffer, int receive_len){
	p_uart->UART_TPR = (uint32_t) data;    // transmit pointer
	p_uart->UART_TCR = send_len;           // transmit counter
	p_uart->UART_RPR = (uint32_t) buffer;  // receive pointer
	p_uart->UART_RCR = receive_len;        // receive counter
	p_uart->UART_PTCR = 0x101;             // transmit and receive enable
	/* Trigger RX and TX */
	p_uart->UART_IER = 0x3;                // set RXRDY and TXRDY
	while ( (p_uart->UART_SR & (1<<3)) == 0 ){} // poll ENDRX flag
}
