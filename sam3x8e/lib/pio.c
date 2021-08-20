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

#include "pio.h"
#include "pmc.h"

/* pio_get_pid()
 *
 * compute the Peripheral ID from the Pio pointer.
 */
int pio_get_pid(Pio* p_pio){
	return (((int)p_pio >> 9) & 0xff) + 4;
}

/* pio_select_input_pins()
 *
 * Configure pins indicated by pinmask as input pins.
 */
void pio_select_input_pins(Pio* p_pio, int pinmask){
	pmc_enable_peripheral_clock(PMC, pio_get_pid(p_pio)); // clock the peripheral
	p_pio->PIO_PER = pinmask; // PIO enable
	p_pio->PIO_ODR = pinmask; // output disable
}

/* pio_select_output_pins()
 *
 * Configure pins indicated by pinmask as output pins.
 */
void pio_select_output_pins(Pio* p_pio, int pinmask){
	p_pio->PIO_PER = pinmask; // PIO enable
	p_pio->PIO_OER = pinmask; // output enable
}

/* pio_select_multidrive_pins()
 *
 * Configure pins indicated by pinmask as multidrive pins.
 * A multidrive pin is open drain instead of high to allow other
 * devices to pull the signal line low.
 */
void pio_select_multidrive_pins(Pio* p_pio, int pinmask){
	p_pio->PIO_PER = pinmask; // PIO enable
	p_pio->PIO_OER = pinmask; // output enable
	p_pio->PIO_MDER = pinmask; // multidrive enable
}

/* pio_select_peripheral_A()
 *
 * Disable PIO and select peripheral A for the port pins
 * indicated by bitmask.
 */
void pio_select_peripheral_A(Pio* p_pio, int pinmask){
	p_pio->PIO_PDR = pinmask; // PIO disable => Peripherals enable
	pmc_enable_peripheral_clock(PMC, pio_get_pid(p_pio)); // clock the peripheral
	p_pio->PIO_ABSR = p_pio->PIO_ABSR & ~pinmask; // select peripheral A
}

/* pio_select_peripheral_B()
 *
 * Disable PIO and select peripheral B for the port pins
 * indicated by bitmask.
 */
void pio_select_peripheral_B(Pio* p_pio, int pinmask){
	p_pio->PIO_PDR = pinmask; // PIO disable => Peripherals enable
	pmc_enable_peripheral_clock(PMC, pio_get_pid(p_pio)); // clock the peripheral
	p_pio->PIO_ABSR = p_pio->PIO_ABSR | pinmask; // select peripheral B
}

/* pio_pullup_enable()
 *
 * Hang the pins indicated by bitmask to an internal pullup resistor.
 */
void pio_pullup_enable(Pio* p_pio, int pinmask) {
	p_pio->PIO_PUER = pinmask;
}

/* pio_set_output_pins()
 *
 * Set the pins indicated by bitmask to high level.
 */
void pio_set_output_pins(Pio* p_pio, int pinmask){
	p_pio->PIO_SODR = pinmask;
}

/* pio_clear_output_pins()
 *
 * Set the pins indicated by bitmask to low level.
 */
void pio_clear_output_pins(Pio* p_pio, int pinmask){
	p_pio->PIO_CODR = pinmask;
}

/* pio_toggle_output_pins()
 *
 * Change state of the pins indicated by bitmask.
 */
void pio_toggle_output_pins(Pio* p_pio, int pinmask){
	int value=(p_pio->PIO_ODSR)^pinmask;
	p_pio->PIO_SODR = value;
	p_pio->PIO_CODR = ~value;
}

/* pio_read_input_pins()
 *
 * Evaluate the input pins of a port and return
 * the resulting uint32_t.
 */
uint32_t pio_read_input_pins(Pio* p_pio) {
	return p_pio->PIO_PDSR;
}
