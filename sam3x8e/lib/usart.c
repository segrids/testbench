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


 *  12      | A | RX2       | Due Pin 17  | USART1 master
 *  13      | A | TX2       | Due Pin 16  | USART1 master

*/
 
#include "usart.h"
#include "pmc.h"
#include "pio.h"


/* usart_flush() 
 *
 * Flush the USART interface. 
 */
void usart_flush(Usart* p_usart) {
	p_usart->USART_CR = 0x3 << 2; // reset receiver and transmitter
}

/* usart_set_rts(USART, value)
 *
 * Set the RTS line to low if value == 0, or high if value == 1.
 * The default state is high.
 */
void usart_set_rts(Usart* p_usart, int value) {
	if (value == 0){
		p_usart->USART_CR = 1<<18;  // setting RTSEN drives RTS low
	} else {
		p_usart->USART_CR = 1<<19;  // setting RTSDIS drives RTS high
	}
}


/* uart_close()
 *
 * No need to close an interface so far.
 */
void usart_close(Usart* p_usart) {
	return;
}


/* usart_configure()
 *
 * Enable USART and set the baudrate, mode = 8N1
 */
void usart_configure(Usart* p_usart, uint32_t mck, uint32_t baud_rate) {
	pmc_enable_peripheral_clock(PMC, 18); // 18 is the PID of USART1, see Datasheet page 38
	pio_select_peripheral_A(PIOA, 0xf << 12);
	uint16_t cd = mck / (baud_rate * 16);
	p_usart->USART_BRGR = cd;
	p_usart->USART_MR = 0xC0; // bit (6,7) = (1,1) => CHRL=3 => char len is 8 bit
	p_usart->USART_CR = 0x50;  // set TXEN and RXEN
}

/* usart_receive_uint8()
 *
 * Receive one character. 
 */
void usart_receive_uint8(Usart* p_usart, uint8_t *p) {
	while ((p_usart->USART_CSR & 0x01) == 0) { }
	*p = (uint8_t)p_usart->USART_RHR;
}

/* usart_send_uint8()
 *
 * Send one character. 
 */
void usart_send_uint8(Usart* p_usart, uint8_t byte) {
	while ((p_usart->USART_CSR & 0x02) == 0) { }
	p_usart->USART_THR = byte;
}


int usart_master_send_data(Usart *p_usart, uint8_t slave_address, uint8_t *buffer, uint32_t buffer_len){
	// HDQ protocol requires most significant bit one for write operations
	for (int i=0; i<buffer_len; i++){
		usart_send_uint8(p_usart, buffer[i]);
		}
	return 0;
}

int usart_master_receive_data(Usart *p_usart, uint8_t slave_address, uint8_t *buffer, uint32_t buffer_len){
	for (int i=0; i<buffer_len; i++){
		usart_receive_uint8(p_usart, buffer+i);
		}
	return 0;
}


