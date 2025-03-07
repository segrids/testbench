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
#include "tester.h"
#include "systick.h"
#include "pmc.h"
#include "pio.h"
#include "uart.h"
#include "spi.h"
#include "rtt.h"
#include "tc.h"
#include "utils.h"
#include "apdu.h"
#include "slave.h"

/* handle_test()
 *
 * Handler for APDUs with CMD = 'T'.
 *
 * The handler will vary but shall always provide the follwing instructions:
 * 
 * 'B' Blink: Implemented for a visual feedback that the device is alive.
 * 
 * INS   |   LC      |     DATA    | LE
 * ----- | --------- | ----------- | ---
 * 'B'   |   0       |   b' '      | 0
 * 
 * 
 */
int handle_test(void) {
	uint16_t status = 0x9000;

	if (apdu.ins == 'B') { // blink
		rtt_init(RTT); // set rtc resolution to 1ms
		utils_blink(5, 100);

	/* Test SPI slave 'o': open, 'W': write, 'R': read
         * use 100.000 bits per second
         * ATTENTION: MISO to MISO, MOSI to MOSI */
	} else if (apdu.ins == 'o') {
		spi_enable(SPI0, 0); // 0=slave mode
		spi_flush_receiver(SPI0);

	} else if (apdu.ins == 'w') {
		status = spi_master_send_data(SPI0, 0, apdu.data, apdu.lc); // slave uses master if

	} else if (apdu.ins == 'r') {
		uint8_t buffer[apdu.le];
		status = spi_master_receive_data(SPI0, 0, buffer, apdu.le); // slave uses master if
		slave_send_data(buffer, apdu.le);

	/* end SPI slave tests */

	/* Test TWI slave 'O': open, 'W': write, 'R': read, 'e': echo
         * use slave_address = 0x11, 100.000 bits per second
         * 	*/
	} else if (apdu.ins == 'O') {
		status = slave_init('I', 0x11, 0);

	} else if (apdu.ins == 'W') {
		slave_send_data(apdu.data, apdu.lc);

	} else if (apdu.ins == 'R') {
		uint8_t buffer[apdu.le];
		slave_receive_data(buffer, apdu.le);
		slave_send_data(buffer, apdu.le);
	/* end TWI slave tests */

	} else if (apdu.ins == 'b') {
		tc_pmc_enable(2);
		tc_nvic_enable(2); // enable TC 2 interrupts
		tc_enable_interrupt(TC2, 4);
		tc_init_capture_mode(TC2, 1000000, 1);
		for(int i = 0; i < 10; i++){
			asm("wfi");
		}
		tc_nvic_disable(29);
		tc_disable_interrupt(TC2, 4);
		tc_pmc_disable(29);

	} else if (apdu.ins == 'C') { /* clear Arduino Due pins 
		 * permitted data values: 7, 9
 		 *	C21      |PIO| out       | Due Pin  9
 		 *	C23      |PIO| out       | Due Pin  7
 		 * use S to set the pins */  
		if (apdu.lc != 1){
			status = 0x6705;
		} else {
			if (apdu.data[0] == 7){	
				pio_clear_output_pins(PIOC, 1<<23);
			} else if (apdu.data[0] == 9){	
				pio_clear_output_pins(PIOC, 1<<21);
			} else {
				status = 0x6706;
			}
		}

	} else if (apdu.ins == 'S') { /* set Arduino Due pins 
		 * permitted data values: 7, 9
 		 *	C21      |PIO| out       | Due Pin  9
 		 *	C23      |PIO| out       | Due Pin  7
 		 * use C to clear the pins */  
		if (apdu.lc != 1){
			status = 0x6703;
		} else {
			if (apdu.data[0] == 7){	
				pio_set_output_pins(PIOC, 1<<23);
			} else if (apdu.data[0] == 9){	
				pio_set_output_pins(PIOC, 1<<21);
			} else {
				status = 0x6704;
			}
		}

	} else if (apdu.ins == 'T') {
		uart_pdc_send(UART, apdu.data, apdu.le);

	} else if (apdu.ins == 'Y') {  // get sYstick value
		slave_send_uint32(systick_get_value(SYSTICK));

	} else {
		perror((void *)handle_test, "Unknown instruction!", 0);
		status = 0x6D00;
	}
	if (status == 0) {
		status = 0x9000;
	}
	slave_send_uint16(status);
}


