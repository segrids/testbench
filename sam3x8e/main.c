/* MIT License

Copyright (c) 2021 by SEGRIDS Gmbh <www.segrids.com>

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

Author: SEGRIDS GmbH <www.segrids.com>
*/
 
#include "pmc.h"
#include "pio.h"
#include "wdt.h"
#include "tc.h"
#include "rtt.h"

#include "utils.h"

#include "apdu.h"
#include "slave.h"

#ifndef BOOTLOADER
#include "adapter.h"
#include "tester.h"
#include "lihandler.h"
#include "crypthandler.h"
#include "debugger.h"
#endif


#ifdef RESPONDER
#include "responder.h"
#endif

#include "loader.h"

   

/* init_pio()
 *
 * Assign sam3x8e pins to a peripheral or port according to 
 *
 * (a) Arduino Due wiring, 
 * (b) project needs:
 *
 *  PIOA
 *	 2      | A | in        | Due Pin A7  | slave select jumper | bridge with GND to select SPI
 *	 3      | A | out       | Due Pin A6  | slave select jumper | bridge with GND to select USART 
 *	 4      | A | in        | Due Pin A5  | slave select jumper | bridge with GND to select I2C
 *	 8      | A | RX        | Due Pin  0  | UART slave   UART2USB
 *	 9      | A | TX        | Due Pin  1  | UART slave   UART2USB
 *	10      | A | RX1       | Due Pin 19  | USART0 slave  UART2BLE
 *	11      | A | TX1       | Due Pin 18  | USART0 slave  UART2BLE
 *	12      | A | RX2       | Due Pin 17  | USART1 master (RXD1)
 *	13      | A | TX2       | Due Pin 16  | USART1 master (TXD1) 
 *	14      | A |           | Due Pin 23  | USART1 master (RTS1)
 *	15      | A |           | Due Pin 24  | USART1 master (CTS1)
 *	16      | A |           | Due AD0     | USART1 master (SCK1)
 *	17      | A | TWD0      | Due SDA1    | TWI0 slave
 *	18      | A | TWCK0     | Due SCL1    | TWI0 slave
 *	25      | A | MISO      | Due SPI     | SPI0 master
 *	26      | A | MOSI      | Due SPI     | SPI0 master
 *	27      | A | SCLK      | Due SPI     | SPI0 master
 *	28 (C29)|PIO| nSS       | Due Pin 10  | SPI0 master: connect to use board as SPI slave
 *	29 (C26)|PIO| don't use | Due Pin  4  | use C26 instead for Debugger SWD data
 *  PIOB
 *  12      | A | TWD1      | Due Pin 20  | TWI1 master
 *  13      | A | TWCK1     | Due Pin 21  | TWI1 master
 *  14      |PIO| out       | Due Pin 53  | RED
 *  25      |   |           | Due Pin 2   |
 *	27      |PIO| out       | Due Pin 13  | LED
 *	28      | A | SWCLK     | SWD / JTAG  | SWD
 *	29      | A | TDI       | JTAG        |
 *	30      | A | TDO       | JTAG        | 
 *	31      | A | TDS/SWDIO | SWD / JTAG  | SWD
 *  PIOC
 *	21      |PIO| out       | Due Pin  9  | GPIO test / or as trigger PIN in measurement
 *	22      |PIO| in        | Due Pin  8  | GPIO test
 *	23      |PIO| out       | Due Pin  7  | GPIO test
 *	24      |PIO| out       | Due Pin  6  | Adapter: boot select
 *	25      |PIO| out       | Due Pin  5  | Debugger and Adapter: nreset target
 *	26 (A29)|PIO|           | Due Pin  4  | Debugger SWD data
 *	28      |PIO| out       | Due Pin  3  | Debugger: Clock
 *	29 (A28)|PIO| dont use  | Due Pin 10  | use A28 instead for SPIO0 nSS
 *  PIOD
 *   4      |   | in        | Due Pin 14  | HDQ Master input: enable pull-up and bridge 
 *          |   |           |             | via 50Ohm with Due Pin 15 (HDQ signal line)
 * 	 5      |   | multidrive| Due Pin 15  | HDQ Master multidrive output (never set high, 
 * 	        |   |           |             | only set low or open drain): HDQ signal line
 * 	 7      |   |           | Due Pin 11  |
 * 	 8      |   |           | Due Pin 12  |
 */
void init_pio(void) {
	// PORT A
	pio_select_peripheral_A(PIOA, 0xF << 25);// SPI
	pio_select_input_pins(PIOA, 7 << 2);     // Arduino Due pins A7,A6,A5 (ANALOG side)
	// PORT B
	pio_select_output_pins(PIOB, 1<<27);     // Arduino Due Pin 13, reserved for LED
	pio_select_output_pins(PIOB, 1<<14);     // Arduino Due Pin 53, reserved for RED
	// PORT C
	pio_select_output_pins(PIOC, 1 << 21);   // Arduino Due pin 9
	pio_select_input_pins(PIOC, 1 << 22);    // Arduino Due pin 8
	pio_select_output_pins(PIOC, 1 << 23);   // Arduino Due pin 7
	pio_select_output_pins(PIOC, 1 << 24);    // Arduino Due pin 6
	pio_select_output_pins(PIOC, 1 << 28);   // 1 << 28 Arduino Due pin 4 reserved for debugger
	pio_select_output_pins(PIOC, 1 << 26);   // 1 << 26 Arduino Due pin 3 reserved for debugger
	pio_select_output_pins(PIOC, 1 << 25);   // 1 << 25 Arduino Due pin 4 reserved for debugger
	pio_set_output_pins(PIOC, 1 << 25);
	// PORT D
	pio_select_input_pins(PIOD, 1 << 4);     // HDQ Master input, Arduino Due PIN 14
	pio_pullup_enable(PIOD, 1 << 4);
	pio_select_multidrive_pins(PIOD, 1 << 5);// HDQ Master output, Arduino Due PIN 15
}


/* select_slave()
 *
 * Evaluate jumper configuration on Arduino Due pins A5, A6, A7 
 * and call the slave_init function to setup the suitable slave interface.
 *
 * Jumper configuration | Slave interface
 * -------------------- | ---------------
 * no jumper            | UART
 * bridge A4 and A5     | USART1 (Due RX2 / 17, TX2 / 16)
 * bridge A5 and A6     | TWI
 * bridge A6 and A7     | SPI (for SPI slave connect A10 to GND)
 *
 * Mapping between Arduino Due and sam3x8e pins:
 *
 * Arduino Due | sam3x8e
 * ----------- | -------
 * A5          | PIOA.4
 * A6          | PIOA.3
 * A7          | PIOA.2
 *
 * */


int select_slave(void) {
	// apply internal pull-up to PIOA.2 (Due A7), PIOA.3 (Due A6), PIOA.4 (Due A5)
	pio_pullup_enable(PIOA, 7<<2);
	// if PIOA.2 (Due A7) is low, select 'S' (SPI)
	// if PIOA.3 (Due A6) is low, select 'V' (USART1)
	// if PIOA.4 (Due A5) is low, select 'I' (I2C / TWI)
	// else select 'U' (UART)
#ifndef BOOTLOADER
	if (((pio_read_input_pins(PIOA) >> 2) & 1) == 0) {
		utils_blink(2, 500);
		return slave_init('S', 0, 0);
	} else if (((pio_read_input_pins(PIOA) >> 3) & 1) == 0) {
		utils_blink(3, 500);
		return slave_init('V', 0, 0);
	} else if (((pio_read_input_pins(PIOA) >> 4) & 1) == 0) {
		utils_blink(4, 500);
		return slave_init('I', 0x11, 0);
	}
#endif
	return slave_init('U', 0, 0);
}

/* handle_apdu()
 *
 * Evaluate the CLA byte of a received APDU (application protocol data unit) and call the suitable command handler.
 */
void handle_apdu(void) {
	if (apdu.lc > 260){
		slave_send_uint16(0x6700);
#ifndef BOOTLOADER
	} else if (apdu.cla == 'A') {
		handle_adapter();
	} else if (apdu.cla == 'C') {
		handle_crypt();
	} else if (apdu.cla == 'I') {
		handle_li();
#ifdef RESPONDER
	} else if (apdu.cla == 'R') {
		handle_responder();
#endif
	} else if (apdu.cla == 'S') {
		handle_swd();
	} else if (apdu.cla == 'T') {
		handle_test();
#endif
	} else if (apdu.cla == 'L') {
		return handle_loader();
	} else {
	        perror((void *)handle_apdu, "Unknown APDU class", apdu.cla);
		slave_send_uint16(0x6C00);
	}
}


/* main() */
void main(void) {
	pmc_24MHz(PMC);    // set master clock to 24MHz
	wdt_enable(WDT, 0);// disable watchdog
	init_pio();        // configure sam3x8e pins
	rtt_init(RTT);     // set timer resolution to 1ms
	select_slave();    // select slave interface (UART, SPI or TWI) according to jumper configuration
	apdu_loop(&handle_apdu); // poll the slave interface for APDUs and execute
	utils_toggle_red();      // never should be here
}



