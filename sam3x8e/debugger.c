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

Author: David Kuehnemann
*/

#include "debugger.h"
#include "pio.h"
#include "swd.h"
#include "slave.h"
#include "apdu.h"

/*
 * handle_swd()
 *
 * Handle APDUs for the instruction class 'S' (SWD).
 *
 * Supported instructions INS: 
 *   'W': switch to SWD, 
 *   '0': read ID code, 
 *   '1': write ID code, 
 *   '2': line reset, 
 *   'R': target reset
 */
int handle_swd(void) {
	uint16_t status = 0x9000;

	if (apdu.ins == 'W') { // switch to SWD
		// switch to SW-DP
		pio_select_output_pins(PIOC, swd_data_pin_c);
		pio_set_output_pins(PIOC, swd_data_pin_c);

		// send more than 50 cycles with data = 1
		pio_set_output_pins(PIOC, swd_data_pin_c);
		for (int i = 0; i < 60; i ++) {
			pio_set_output_pins(PIOC, swd_clock_pin_c);
			pio_clear_output_pins(PIOC, swd_clock_pin_c);
		}

		// send 16-bit sequence 0x79E7 (most significant bit first)
		for (int i = 15; i >= 0; i--) {
			if (0x79E7 & (1 << i)) {
				pio_set_output_pins(PIOC, swd_data_pin_c);
			} else {
				pio_clear_output_pins(PIOC, swd_data_pin_c);
			}
			pio_set_output_pins(PIOC, swd_clock_pin_c);
			pio_clear_output_pins(PIOC, swd_clock_pin_c);
		}

		// send more than 50 cycles with data = 1
		//pio_set_output_pins(PIOC, swd_data_pin_c);
		//for (int i = 0; i < 60; i++) {
		//	pio_set_output_pins(PIOC, swd_clock_pin_c);
		//	pio_clear_output_pins(PIOC, swd_clock_pin_c);
		//}

		// pull data low
		pio_clear_output_pins(PIOC, swd_data_pin_c);
		pio_set_output_pins(PIOC, swd_clock_pin_c);
		pio_clear_output_pins(PIOC, swd_clock_pin_c);

		// cycles without data (startup time?)
		for (int i = 0; i < 20; i++) {
			pio_set_output_pins(PIOC, swd_clock_pin_c);
			pio_clear_output_pins(PIOC, swd_clock_pin_c);
		}

		// read IDCODE
		uint32_t value = 0;
		int err = swd_read(0xa5, &value);
		slave_send_uint32(value);

		status = err;

	} else if (apdu.ins == '0') {
		uint8_t req = *(uint8_t *) apdu.data;

		// read IDCODE 0xa5
		uint32_t value = 0;
		int err = swd_read(req, &value);
		slave_send_uint32(value);
		//pio_set_output_pins(PIOC, swd_clock_pin_c);
		//pio_clear_output_pins(PIOC, swd_data_pin_c);

		status = err;

	} else if (apdu.ins == '1') {
		// write IDCODE
		uint32_t value = *(uint32_t *) apdu.data;
		uint8_t req = *(uint8_t *) (apdu.data + 4);
		int err = swd_write(req, value);

		status = err;

	} else if (apdu.ins == '2') {
		// line reset
		swd_line_reset();

	} else if (apdu.ins == 'R') {
		// target reset
		swd_target_reset(); // pulls Due pin 5 for a while
	} else {
		perror((void *)handle_swd, "Unknown instruction!", 0);
		status = 0x6D00;
	}

	slave_send_uint16(status);
	return 0;
}

