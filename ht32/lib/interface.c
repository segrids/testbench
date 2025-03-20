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
 
#include "usart.h"
#include "i2c.h"
#include "gpio.h"

#include "interface.h"

interface_t interface;

static uint8_t response_buffer[255];
static int index;

static const uint8_t pin_select = 1 << 1;

int interface_select(void) {
	gpio_select_input_pins(GPIOC, pin_select);

	if (gpio_read_input_pins(GPIOC) & pin_select) {
		// Use I2C if C1=high.
		interface.pointer = (void *) I2C0;
		interface.receive_apdu = (int (*)(void *)) &i2c_slave_receive_apdu;
		interface.send_uint8 = (int (*)(void *, uint8_t)) 0;
		interface.flush_response = (int (*)(void *, uint8_t *, int)) &i2c_slave_send_apdu_response;
	} else {
		// Use USART if C1=low.
		interface.pointer = (void *) USART1;
		interface.receive_apdu = (int (*)(void *)) &usart_receive_apdu;
		interface.send_uint8 = (int (*)(void *, uint8_t)) &usart_send_uint8;
		interface.flush_response = (int (*)(void *, uint8_t *, int)) &usart_send_apdu_response;
	}
	return 0;
}

int interface_send_uint8(uint8_t x) {
	if (interface.send_uint8 != 0) {
		return interface.send_uint8(interface.pointer, x);
	} else {
		if ((index + sizeof(uint8_t)) < 255) {
			response_buffer[index] = x;
			index++;
			return 0;
		} else {
			return 1;
		}
	}
}

int interface_send_uint16(uint16_t x) {
	if ((index + sizeof(uint16_t)) < 255) {
		interface_send_uint8(x & 0xff);
		interface_send_uint8(x >> 8);
		return 0;
	} else {
		return 1;
	}
}

int interface_send_uint32(uint32_t x) {
	if ((index + sizeof(uint32_t)) < 255) {
		interface_send_uint8(x & 0xff);
		interface_send_uint8((x >> 8) & 0xff);
		interface_send_uint8((x >> 16) & 0xff);
		interface_send_uint8((x >> 24) & 0xff);
		return 0;
	} else {
		return 1;
	}
}

void interface_apdu_loop(int (*handle)(void)) {
	while (1) {
		index = 0;
		(*(interface.receive_apdu))(interface.pointer);
		(*handle)();
		(*(interface.flush_response))(interface.pointer, response_buffer, index);
	}
}
