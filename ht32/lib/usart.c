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

#include "usart.h"

/** Set the UART baudrate ti 115200. **/
void usart_configure(Usart *usart) {
	usart->CR = (1 << 8) | (3 << 4); // 8N1 | Rx and TX enable
	usart->DLR = 69; // BDR=69
}


/** Send one character. **/
void usart_send_uint8(Usart *usart, uint8_t byte){
	while((usart->SIFR & (1<<8)) == 0){
		// wait for Tx complete
	}
	usart->DR = (uint32_t)byte;
}

/** Send a uint16_t in two characters, LSB first. **/
void usart_send_uint16(Usart *usart, uint16_t halfword){
	usart_send_uint8(usart, halfword);
	usart_send_uint8(usart, halfword >> 8);
}

/** Send a uint32_t in four characters, LSB first. **/
void usart_send_uint32(Usart *usart, uint32_t word){
	usart_send_uint16(usart, word);
	usart_send_uint16(usart, word >> 16);
}

/** Receive one character. **/
void usart_receive_uint8(Usart *usart, uint8_t *p) {
	while((usart->SIFR & (1<<0)) == 0){
		// wait for received byte
	}
	*p = (uint8_t)(usart->DR);
}

/** Receive two characters encoding a uint16_t, LSB first. **/
void usart_receive_uint16(Usart *usart, uint16_t *p) {
	usart_receive_uint8(usart, (uint8_t *) p);
	usart_receive_uint8(usart, ((uint8_t *) p) + 1);
}

/**
Receive four characters encoding a uint32_t, LSB first.
special feature (not sure if required for ht32):
buffers uint32_t in a local variable and word write it to the target location.
**/
void usart_receive_uint32(Usart *usart, uint32_t *p) {
	uint32_t word;
	usart_receive_uint16(usart, (uint16_t *)(&word));
	usart_receive_uint16(usart, (uint16_t *)(&word) + 1);
	*p = word;
}

// APDU
int usart_receive_apdu(Usart *usart) {
	usart_receive_uint8(usart, &apdu.cla);
	usart_receive_uint8(usart, &apdu.ins);
	usart_receive_uint16(usart, &apdu.lc);
	if (apdu.lc > 260){
		return 0x6700; //TODO: Error code
	}
	for (uint16_t i = 0; i < apdu.lc; i++) {
		usart_receive_uint8(usart, apdu.data + i);
	}
	usart_receive_uint16(usart, &apdu.le);
	return 0;
}

int usart_send_apdu_response(Usart *usart, uint8_t *buffer, int len) {
	return 0;
}
