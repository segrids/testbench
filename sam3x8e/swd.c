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

#include "swd.h"
#include "pio.h"
#include "rtt.h"

/* Serial Wire Debug (SWD) driver functions */

const uint32_t swd_clock_pin_c = 1 << 28;
const uint32_t swd_data_pin_c = 1 << 26;
const uint32_t swd_reset_pin_c = 1 << 25;

void swd_target_reset(void){
    pio_clear_output_pins(PIOC, swd_reset_pin_c);
    rtt_sleep(RTT, 50);
    pio_set_output_pins(PIOC, swd_reset_pin_c);
}


void swd_line_reset(void) {
    pio_clear_output_pins(PIOC, swd_clock_pin_c);

    pio_select_output_pins(PIOC, swd_data_pin_c);
    pio_set_output_pins(PIOC, swd_data_pin_c);

    for (int i = 0; i < 50; i++) {
        pio_set_output_pins(PIOC, swd_clock_pin_c);
        pio_clear_output_pins(PIOC, swd_clock_pin_c);
    }

    pio_clear_output_pins(PIOC, swd_data_pin_c);

    pio_set_output_pins(PIOC, swd_clock_pin_c);
    pio_clear_output_pins(PIOC, swd_clock_pin_c);
    pio_set_output_pins(PIOC, swd_clock_pin_c);
    pio_clear_output_pins(PIOC, swd_clock_pin_c);
}

int swd_read(uint8_t req, uint32_t * value) {
	pio_select_output_pins(PIOC, swd_data_pin_c);

	for (int i = 7; i >= 0; i--) {
		if (req & (1 << i)) {
			pio_set_output_pins(PIOC, swd_data_pin_c);
		} else {
			pio_clear_output_pins(PIOC, swd_data_pin_c);
		}
		pio_set_output_pins(PIOC, swd_clock_pin_c);
		pio_clear_output_pins(PIOC, swd_clock_pin_c);
	}

	pio_clear_output_pins(PIOC, swd_data_pin_c);

	// TODO: move input pin clock enable to a seperate *_init function
	pio_select_input_pins(PIOC, swd_data_pin_c);

	// turnaround
	pio_set_output_pins(PIOC, swd_clock_pin_c);
	pio_clear_output_pins(PIOC, swd_clock_pin_c);


	// ACK? TODO: return 2/3 if WAIT/FAULT
	uint32_t ack = 0;
	for (int i = 0; i < 3; i++) {
		if (pio_read_input_pins(PIOC) & swd_data_pin_c) {
			// 1
			ack = ack | (1 << i);
		} else {
			// 0
		}
		pio_set_output_pins(PIOC, swd_clock_pin_c);
		pio_clear_output_pins(PIOC, swd_clock_pin_c);
	}

	switch (ack) {
		case 1:
			// OK
			break;
		case 2:
			// WAIT
			return 2;
		case 4:
			// FAULT
			perror((void *)swd_read, "Error in swd_read: ACK=4", (int)req);
			return 4;
                default:
                        // protocol error
	                perror((void *)swd_read, "Protocol error in swd_read", ack);
                        return ack;
	}

	int parity = 0;
	*value = 0;
	// read response
	for (int i = 0; i < 32; i++) {
		if (pio_read_input_pins(PIOC) & swd_data_pin_c) {
			// 1
			parity = !parity;
			*value = *value | (1 << i);
		} else {
			// 0
		}
		pio_set_output_pins(PIOC, swd_clock_pin_c);
		pio_clear_output_pins(PIOC, swd_clock_pin_c);
	}

	// convert to booleans before comparing
	int parity_matches = !parity == !((pio_read_input_pins(PIOC) & swd_data_pin_c));

	pio_set_output_pins(PIOC, swd_clock_pin_c);
	pio_clear_output_pins(PIOC, swd_clock_pin_c);

	// data direction changes: turnaround
    pio_set_output_pins(PIOC, swd_clock_pin_c);
    pio_clear_output_pins(PIOC, swd_clock_pin_c);

	if (parity_matches) {
		return 9; // success
	} else {
	        perror((void *)swd_read, "Parity mismatch in swd_read", (int)req);
		return 8; // parity mismatch
	}
}

int swd_write(uint8_t req, uint32_t value) {
	pio_select_output_pins(PIOC, swd_data_pin_c);

	for (int i = 7; i >= 0; i--) {
		if (req & (1 << i)) {
			pio_set_output_pins(PIOC, swd_data_pin_c);
		} else {
			pio_clear_output_pins(PIOC, swd_data_pin_c);
		}
		pio_set_output_pins(PIOC, swd_clock_pin_c);
		pio_clear_output_pins(PIOC, swd_clock_pin_c);
	}

	// TODO: move input pin clock enable to a seperate *_init function
	pio_select_input_pins(PIOC, swd_data_pin_c);

	// turnaround
	pio_set_output_pins(PIOC, swd_clock_pin_c);
	pio_clear_output_pins(PIOC, swd_clock_pin_c);

	uint32_t ack = 0;
	for (int i = 0; i < 3; i++) {
		if (pio_read_input_pins(PIOC) & swd_data_pin_c) {
			// 1
			ack = ack | (1 << i);
		} else {
			// 0
		}
		pio_set_output_pins(PIOC, swd_clock_pin_c);
		pio_clear_output_pins(PIOC, swd_clock_pin_c);
	}

	switch (ack) {
		case 1:
			// OK
			break;
		case 2:
			// WAIT
			return 3;
		case 4:
			// FAULT
			perror((void *)swd_write, "Error in swd_write: ACK=4", (int)value);
			return 1;
	}

	// turnaround happens before data transfer on write
	pio_set_output_pins(PIOC, swd_clock_pin_c);
	pio_clear_output_pins(PIOC, swd_clock_pin_c);

	pio_select_output_pins(PIOC, swd_data_pin_c);

	int parity = 0;
	for (int i = 0; i < 32; i++) {
		if (value & (1 << i)) {
			parity = !parity;
			pio_set_output_pins(PIOC, swd_data_pin_c);
		} else {
			pio_clear_output_pins(PIOC, swd_data_pin_c);
		}
		pio_set_output_pins(PIOC, swd_clock_pin_c);
		pio_clear_output_pins(PIOC, swd_clock_pin_c);
	}

	if (parity) {
		pio_set_output_pins(PIOC, swd_data_pin_c);
	} else {
		pio_clear_output_pins(PIOC, swd_data_pin_c);
	}

	pio_set_output_pins(PIOC, swd_clock_pin_c);
	pio_clear_output_pins(PIOC, swd_clock_pin_c);

	// clear data before turnaround
	pio_clear_output_pins(PIOC, swd_data_pin_c);

    // clock for at least 8 more times to prevent timeout
    for (int i = 0; i < 8; i++) {
        pio_set_output_pins(PIOC, swd_clock_pin_c);
        pio_clear_output_pins(PIOC, swd_clock_pin_c);
    }

	return 0;
}
