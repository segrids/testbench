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
 
/* main.c
 *
 * USART1: As discribed in Table 3 of HT32F52342/HT32F52352 Datasheet,
 *	- AF6 of PA4 is USR1_TX
 *	- AF6 of PA5 is USR1_RX
 * USART1 clock:
 *	- If APBPCSR0.USR1PCLK = 0 then CK_USART1 = CK_AHB
 *	- If ... then CK_AHB == CK_SYS
 *	- If GCCR.SW == 2 then CK_SYS == HSE (8MHz)
 * Baud Rate Clock = CK_USART1 / BRD
 *	- With BRD = 69, we have a Deviation Error rate of 0.64%
 */


#include "types.h"
#include "ckcu.h"
#include "usart.h"
#include "apdu.h"
#include "gpio.h"
#include "crc.h"
#include "i2c.h"
#include "interface.h"
#ifdef CRYPT
#include "crypthandler.h"
#endif

int handle_apdu(void);
int handle_responder(void);
//int handle_swd(void);
int handle_loader(void);
int handle_qwiic(void);


/* --------- SCB --------------------------------------- */
uint32_t * const CPUID = (uint32_t *) 0xE000ED00;
uint32_t * const CHIPID = (uint32_t *) 0x40080180;
uint32_t * const CUSTOMID = (uint32_t *) 0x40080310;

/* --------- AFIO --------------------------------------- */
uint32_t * const GPACFGLR = (uint32_t *) 0x40022020; //GPIO Port A AFIO Configuration Register 0
uint32_t * const GPACFGHR = (uint32_t *) 0x40022024; //GPIO Port A AFIO Configuration Register 0
uint32_t * const GPBCFGLR = (uint32_t *) 0x40022028; //GPIO Port B AFIO Configuration Register 0
uint32_t * const GPBCFGHR = (uint32_t *) 0x4002202c; //GPIO Port B AFIO Configuration Register 0
uint32_t * const GPCCFGLR = (uint32_t *) 0x40022030; //GPIO Port C AFIO Configuration Register 0
uint32_t * const GPCCFGHR = (uint32_t *) 0x40022034; //GPIO Port C AFIO Configuration Register 0
uint32_t * const GPDCFGLR = (uint32_t *) 0x40022038; //GPIO Port D AFIO Configuration Register 0
uint32_t * const GPDCFGHR = (uint32_t *) 0x4002203c; //GPIO Port D AFIO Configuration Register 0
/* --------- GPIO --------------------------------------- */
/* GPIOA at 0x400B0000 */
uint32_t * const PADIRCR = (uint32_t *) 0x400B0000;
uint32_t * const PADOUTR = (uint32_t *) 0x400B0020;
/* GPIOC at 0x400B4000 */
uint32_t * const PCDIRCR = (uint32_t *) 0x400B4000;
uint32_t * const PCDOUTR = (uint32_t *) 0x400B4020;


void set_led(uint8_t value){
	if (value == 0){
		gpio_set_output_pins(GPIOC, 3 << 14);
	} else if (value == 1){
		gpio_clear_output_pins(GPIOC, 3 << 14); // set PC14-15
	} else {
		gpio_toggle_output_pins(GPIOC, 3 << 14);
	}
}

/* Fixed Pin configuration for the ESK32-30501 V2.0 Starter Kit

- USART1 Slave
- I2C Slave
- I2C Master
- SPI Slave


See Datasheet Page 29 (of 50):

 Board Pin        | HT32 Pin  | AF | Usage
 ---------------- | --------- | -- | -----------------
 A0               | PA0  (1)  | 7  | I2C1 SCL - I2C Master (QWIIC)
 A1               | PA1  (2)  | 7  | I2C1 SDA - I2C Master (QWIIC)
 A2               | PA2  (3)  |    | Trigger Out 0
 A3               | PA3  (4)  |    | Trigger Out 1
 A4               | PA4  (5)  | 6  | USART1 RX
 A5 / J2 RS232_Tx |           |    |           (USART1 TX if jumper J2 is inserted)
 J2-PA5           | PA5  (6)  | 6  | USART1 TX (or jumper)
 A6               | PA6  (7)  |    | 
 A7               | PA7  (8)  |    | 
 A8               | PA8  (34) |    | 
 A9               | PA9  (35) |    | Boot Select 0=ISP, 1=Flash 
 A10              | PA10 (36) |    | 
 A11              | PA11 (37) |    | 
 SWCLK            | PA12 (38) |    | SWD Debug CLK
 SWDIO            | PA13 (39) |    | SWD Debug IO 
 A14              | PA14 (40) |    | 
 A15              | PA15 (41) |    | 
 ---------------- | --------- | -- | -----------------
 B0               | PB0  (44) |    | 
 B1               | PB1  (45) |    | 
 B2               | PB2  (49) |    | 
 B3               | PB3  (50) |    | 
 B4               | PB4  (51) |    | 
 B5               | PB5  (52) |    | 
 B6               | PB6  (60) |    | 
 B7               | PB7  (61) |    | 
 B8               | PB8  (62) |    | 
 XX               | XXXX (XX) |    | 
 B10              | PB10 (22) |    | 32768 Hz Quarz X1
 B11              | PB11 (23) |    | 32768 Hz Quarz X1
 B12              | PB12 (24) |    | RTC OUT 
 B13              | PB13 (26) |    | 8 MHz Quarz Y2
 B14              | PB14 (27) |    | 8 MHz Quarz Y2
 B15              | PB15 (28) |    | 
 ---------------- | --------- | -- | -----------------
 C0               | PC0  (29) |    | GPIO Output
 C1               | PC1  (57) |    | Slave Select 0=USART, 1=I2C (ohne Jumper)
 C2               | PC2  (58) |    | 
 C3               | PC3  (59) |    | 
 C4               | PC4  (11) |    | 
 C5               | PC5  (12) |    | 
 C6               | PC6  (15) |    | USBDM 
 C7               | PC7  (16) |    | USBDP
 C8               | PC8  (13) |    | 
 C9               | PC9  (14) |    | 
 C10              | PC10 (30) |    | 
 C11              | PC11 (31) |    | 
 C12              | PC12 (32) | 7  | I2C0 SCL  - I2C Slave
 C13              | PC13 (33) | 7  | I2C0 SDA  - I2C Slave
 C14              | PC14 (53) |    | LEDs D7-D9
 C15              | PC15 (54) |    | LEDs D7-D9
 ---------------- | --------- | -- | -----------------
 D0               | PD0 (25)  |    | 
 D1               | PD1 (46)  |    | 
 D2               | PD2 (47)  |    | 
 D3               | PD3 (48)  |    | 
*/

void main(void){
	ckcu_configure();

	/* ------------ AFIO Configuration ----------------------- 
        See Table 22 on page 177 (of 656) of User Manual
        and Table 3  on page 27 (of 50) of Datasheet.
    */
	
	// PA7->AF0
	// PA6->AF0
	// PA5->AF6 (USART1)
	// PA4->AF6 (USART1)
	// PA3->AF1 (PA3)
	// PA2->AF1 (PA2)
	// PA1->AF7 (I2C1 - SDA)
	// PA0->AF7 (I2C1 - SCL)
	*GPACFGLR = 0x660077 << 0;
	// PC13->AF7 (I2C0 - SDA)
	// PC12->AF7 (I2C0 - SCL)
	*GPCCFGHR = 0x77 << 16;

	/* ------------ GPIO Configuration------------------------ */
	gpio_select_output_pins(GPIOA, 3 << 2);  // configure PA2-PA3 as output pin 
	gpio_select_output_pins(GPIOC, 1 << 0);  // configure PC0 as output pin 
	gpio_select_input_pins(GPIOC, 1 << 1);   // configure PC1 as input pin 
	gpio_select_output_pins(GPIOC, 3 << 14); // configure PC14-15 as output pins 

	/* ------------ USART1 Configuration------------------------ */
	usart_configure(USART1);
	/* ------------ ------------------------ */


	/* ------------ main ------------------------ */
	gpio_clear_output_pins(GPIOC, 1 << 0); // set PC0

	i2c_configure(I2C0);
	i2c_slave_set_address(I2C0, 0x37);

	interface_select();
	interface_apdu_loop(&handle_apdu);
}

int handle_apdu(void) {
	if (apdu.lc > 260){
		interface_send_uint16(0x6700);
	} else if (apdu.cla == 'L') { 
		return handle_loader();
#ifdef RESPONDER
	} else if (apdu.cla == 'R') {
		return handle_responder();
#endif
#ifdef CRYPT
	} else if (apdu.cla == 'C') {
		return handle_crypt();
#endif
#ifdef QWIIC
	} else if (apdu.cla == 'Q') {
		return handle_qwiic();
#endif
	} else {
		interface_send_uint16(0x6C00);
	}
	return 0;
}

#ifdef RESPONDER
uint32_t * const PROGRAM_SRC  = (uint32_t *) 0x00010000;
uint32_t * const PROGRAM_DEST = (uint32_t *) 0x20000000;

int handle_responder(void) {
	uint16_t status = 0x9000;

	if (apdu.ins == 'C') {
		uint32_t module_word_len = (PROGRAM_SRC[0]) / 4;
		uint32_t matrix_word_len = (PROGRAM_SRC[1]) / 4;

		for (int i = 0; i < module_word_len; i++) {
			// to be replaced by swapping
			PROGRAM_DEST[i] = PROGRAM_SRC[i+2];
		}
		// copy the 16 Byte challenge to matrix
		uint32_t * data = (uint32_t *) apdu.data;
		for (int i = 0; i < 4; i++) {
			PROGRAM_DEST[module_word_len + i] = data[i];
		}
		for (int i = module_word_len + 4; i < module_word_len + 4 + matrix_word_len; i++) {
			// to be replaced by braiding
			PROGRAM_DEST[i] = PROGRAM_SRC[i-2];
		}
		for (int i = module_word_len + 4 + matrix_word_len; i < module_word_len + 8 + matrix_word_len; i++) {
			// need zeros as stop condition
			PROGRAM_DEST[i] = 0;
		}

		void (*thread)(void) = (void (*)(void)) ((uint8_t *) PROGRAM_DEST + 0xc1);
		// launch thread code
		thread();
		uint8_t * pmstack = (uint8_t *) 0x20003a00;
		// send the response from _smstack
		for (int i = 0; i < apdu.le; i++) {
			interface_send_uint8(pmstack[i]);
		}
	} else if (apdu.ins == 'I') {
		uint32_t w = *CHIPID;
		interface_send_uint32(w);
		w = *CPUID;
		interface_send_uint32(w);
		for (int i=0; i<4; i++){
			w = CUSTOMID[i];
			interface_send_uint32(w);
		}

	}
	interface_send_uint16(status);
	return 0;
}
#endif

int handle_loader(void) {
	//uint16_t i, value;
	uint16_t status = 0x9000;
	if (apdu.ins == 'D' ) { // data write
		uint32_t address = *(uint32_t *) apdu.data;
		uint8_t * p = (uint8_t *) address;
		uint8_t * data = (uint8_t *) apdu.data + 4;
		for (int i = 0; i < apdu.lc - 4; i++) {
			p[i] = data[i];
		}

	} else if (apdu.ins == 'E') {
		for (int i = 0; i < apdu.le; i++) {
			interface_send_uint8(apdu.data[i]);
		}

	} else if (apdu.ins == 'G' ) { // go to a void function with void argument
		uint32_t address = *(uint32_t *) apdu.data;
		address |= 1;
		void (*fp)(void) = (void (*)(void)) address;
		fp();

	} else if (apdu.ins == 'I') { // integer write
		if (apdu.lc % 4 != 0){
			status = 0x6701;
		} else {
			uint32_t address = *(uint32_t *) apdu.data;
			uint32_t * p = (uint32_t *) address;

			uint32_t * data = (uint32_t *) apdu.data + 1;

			for (int i = 0; i < apdu.lc / 4 - 1; i++) {
				p[i] = data[i];
			}
		}

	} else if (apdu.ins == 'R') { // read
		// note that registers in the AHB bus support only word access
		uint32_t address = *(uint32_t *) apdu.data;
		uint8_t * p = (uint8_t *) address;
		uint32_t * p32 = (uint32_t *) address;
		volatile uint32_t value;
		for (int i = 0; i < apdu.le / 4; i++){
			value = p32[i];
			interface_send_uint32(value);
		}
		for (int i = (apdu.le / 4)*4; i < apdu.le; i++) {
			interface_send_uint8(p[i]);
		}

	} else if (apdu.ins == 'l') {
		uint8_t value = *apdu.data;
		set_led(value);

	} else if (apdu.ins == 'c') { // CRC test
		uint32_t address = *(uint32_t *) apdu.data;
		uint32_t length = *((uint32_t *) apdu.data + 1);

		uint16_t digest = crc_ccitt(CRC, (uint32_t *) address, length);
		interface_send_uint16(digest);
	} else {
		status = 0x6D00;
	}

	interface_send_uint16(status);
	return 0;
}

#ifdef QWIIC
int handle_qwiic(void) {
	uint16_t status = 0x9000;
	if (apdu.ins == 'I' ) {
		if (apdu.lc) {
			i2c_write_to(I2C1, 0x37);
			for (int i = 0; i < apdu.lc; i++) {
				i2c_write_byte(I2C1, apdu.data[i]);
			}
			i2c_send_stop(I2C1);
		}

		if (apdu.le) {
			i2c_read_from(I2C1, 0x37);
			for (int i = 0; i < apdu.le; i++) {
				uint8_t byte = i2c_read_byte(I2C1);
				interface_send_uint8(byte);
			}
			i2c_send_stop(I2C1);
		}
	} else if (apdu.ins == 'E') { // Echo up to 10 (9?) bytes.
		 i2c_slave_set_address(I2C0, 0x37);
		 uint8_t b[10];
		 int n = 0;
		 do {
		     n = i2c_slave_receive(I2C0, b, 10);
		 } while (n == 0);
		 i2c_slave_transmit(I2C0, b, n);
	} else {
		status = 0x6D00;
	}

	interface_send_uint16(status);
	return 0;
}
#endif

