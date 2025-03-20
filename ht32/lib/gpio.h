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
 
#ifndef GPIO_H_
#define GPIO_H_

#include "types.h"

typedef struct {
	volatile uint32_t DIRCR;
	volatile uint32_t INER;
	volatile uint32_t PUR;
	volatile uint32_t PDR;
	volatile uint32_t ODR;
	volatile uint32_t DRVR;
	volatile uint32_t LOCKR;
	volatile const uint32_t DINR;
	volatile uint32_t DOUTR;
	volatile uint32_t SRR;
	volatile uint32_t RR;
} Gpio;

#define GPIOA ((Gpio *) 0x400B0000)
#define GPIOB ((Gpio *) 0x400B2000)
#define GPIOC ((Gpio *) 0x400B4000)
#define GPIOD ((Gpio *) 0x400B6000)

uint32_t gpio_read_input_pins(Gpio* gpio);
void gpio_select_input_pins(Gpio* gpio, uint32_t pinmask);
void gpio_select_output_pins(Gpio* gpio, uint32_t pinmask);
void gpio_set_output_pins(Gpio* gpio, uint32_t pinmask);
void gpio_clear_output_pins(Gpio* gpio, uint32_t pinmask);
void gpio_toggle_output_pins(Gpio* gpio, uint32_t pinmask);
void toggle(void);
void set_trigger(int port, int value);
#endif
