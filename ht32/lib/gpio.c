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
 
#include "gpio.h"

uint32_t gpio_read_input_pins(Gpio* gpio) {
	return gpio->DINR;
}

void gpio_select_input_pins(Gpio* gpio, uint32_t pinmask) {
	gpio->DIRCR &= ~pinmask;
	gpio->INER |= pinmask;
}

void gpio_select_output_pins(Gpio* gpio, uint32_t pinmask) {
	gpio->DIRCR |= pinmask;
}

void gpio_clear_output_pins(Gpio* gpio, uint32_t pinmask) {
	gpio->DOUTR &= ~pinmask;
}

void gpio_set_output_pins(Gpio* gpio, uint32_t pinmask) {
	gpio->DOUTR |= pinmask & 0xffff;
}

void gpio_toggle_output_pins(Gpio* gpio, uint32_t pinmask) {
	gpio->DOUTR ^= pinmask;
}

void toggle(void){
	gpio_toggle_output_pins(GPIOC, 1 << 0); // toggle PC0
}

/*
 port | Starter Kit pin
 ---- | ---------------
 0    | A2
 1    | A3
*/
void set_trigger(int port, int value){
	uint32_t pinmask = 1 << (2 + port);
	if (value == 1){
		gpio_set_output_pins(GPIOA, pinmask);
	} else {
		gpio_clear_output_pins(GPIOA, pinmask);
	}
}
