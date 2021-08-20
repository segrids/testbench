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
 
#include "twi.h"
#include "pmc.h"
#include "pio.h"

/* twi_enable()
 *
 * Initialize TWI0 as slave, TWI1 as master 
 * Pins:
 *      A17      | A | TWD0      | Due SDA1    | I2C slave
 *      A18      | A | TWCK0     | Due SCL1    | I2C slave
 *      B12      | A | TWD1      | Due Pin 20  | I2C master
 *      B13      | A | TWCK1     | Due Pin 21  | I2C master
 */
void twi_enable(Twi* p_twi){
	if (p_twi == TWI0){
		pmc_enable_peripheral_clock(PMC, 22); // TWI0
		pio_select_peripheral_A(PIOA, 3<<17);
	} else if (p_twi == TWI1){ 
		pmc_enable_peripheral_clock(PMC, 23); // TWI1
		pio_select_peripheral_A(PIOB, 3<<12);
	}
}

/* twi_set_slave_mode()
 *
 * Configure TWI as slave with given address.
 */
void twi_set_slave_mode(Twi* p_twi, uint8_t slave_address){
	// The following fields must be programmed before entering Slave mode:
	// 1. SADR (TWI_SMR): The slave device address is used in order to be accessed by 
	//    master devices in read or write mode.
	// 2. MSDIS (TWI_CR): Disable the master mode.
	// 3. SVEN (TWI_CR): Enable the slave mode.
	// As the device receives the clock, values written in TWI_CWGR are not taken into account. */
        // Relevant bits of TWI_CR: 3=MDIS, 4=SVEN             */
	p_twi->TWI_SMR = slave_address << 16;
	p_twi->TWI_CR = 1 << 3;
	p_twi->TWI_CR = 1 << 4;
}

/* twi_set_master_mode()
 *
 * Configure TWI as master selecting one out of two configurations:
 *   - for config=1, set f_SCL=100kHz, 
 *   - for config=4, set f_SCL=400kHz.
 */
int twi_set_master_mode(Twi* p_twi, uint8_t config){
	p_twi->TWI_CR = 1 << 5; // disable slave mode
	// Set clock approx. to fast mode (400kHz).
 	// TMCK = 1/MCK = 1s/24000000
	// set CLDIV=CHDIV=DIV:=7, CKDIV:=3  => with oscillocope: 1 clock cycle = 2.5 us => 400kHz
	//
	//
	int DIV, CKDIV;
	if (config == 1){          // 29 * 2^2 + 4 = 120
		DIV = 29;
		CKDIV = 2;
	} else if (config == 4) {  // 13 * 2^1 + 4 = 30
		DIV = 13;
		CKDIV = 1;
	} else {
		return 0xBCF0;
	}
	p_twi->TWI_CWGR = (CKDIV << 16)|(DIV << 8) | (DIV << 0);
	p_twi->TWI_CR = 1 << 2; // enable master mode
	return 0;
}


/* twi_slave_receive_uint8()
 *
 * Slave receive method for TWI.
 */
int twi_receive_uint8(void* pointer, uint8_t* byte){
	Twi *p_twi = (Twi *)(pointer);
	while ((p_twi->TWI_SR & (1<<1)) == 0){} // poll RXRDY
	*byte = p_twi->TWI_RHR;
	return 0;
}

/* twi_slave_send_uint8()
 *
 * Slave send method for TWI.
 */
int twi_send_uint8(void* pointer, uint8_t byte){
	Twi *p_twi = (Twi *)(pointer);
	while ((p_twi->TWI_SR & (1<<2)) == 0){} // poll TXRDY 
	p_twi->TWI_THR = byte;
	return 0;
}

/* twi_flush()
 *
 * Wait_for_transaction_complete
 */
int twi_flush(void* pointer){
	Twi *p_twi = (Twi *)(pointer);
 	// wait_for_transaction_complete
	while ((p_twi->TWI_SR & (1 << 11)) == 0){} // poll EOSACC
	while ((p_twi->TWI_SR & (1 << 0)) == 0){} // poll TXCOMP
	return 0;
}

void twi_close(void* pointer){
	// TODO
}


/* twi_master_send_data()
 *
 * Master send method for TWI.
 */
int twi_master_send_data(Twi* p_twi, uint8_t slave_address, uint8_t* data, int length){
	// See Flowchart on page 721 of datasheet.     *
 	// Relevant bits of TWI_SR: 0=TXCOMP, 2=TXRDY  *
	// Relevant bits of TWI_CR: 1=STOP             */
	p_twi->TWI_MMR = (slave_address << 16) | (0 << 12); // set slave address, and clear master read direction bit 	
	for (int i=0; i<length; i++){
		p_twi->TWI_THR = data[i];
		while ((p_twi->TWI_SR & (1 << 2)) == 0){} // poll TXRDY
	}
	p_twi->TWI_CR = 1<<1;    // send stop condition
	while ((p_twi->TWI_SR & (1<<0)) == 0){} // poll TXCOMP
	return 0;
}

/* twi_master_receive_data()
 *
 * Master receive method for TWI.
 */
int twi_master_receive_data(Twi* p_twi, uint8_t slave_address, uint8_t* data, int length){
	// See Flowchart on page 724 of datasheet.     *
	// Relevant bits of TWI_SR: 0=TXCOMP, 1=RXRDY  *
	// Relevant bits of TWI_CR: 0=START, 1=STOP             */
	p_twi->TWI_MMR = (slave_address << 16) | (1 << 12); // set slave address, and set master read direction bit 	
	p_twi->TWI_CR = 1<<0;    // send start condition
	for (int i=0; i<length-1; i++){
		while ((p_twi->TWI_SR & (1<<1)) == 0){} // poll RXRDY
		data[i] = p_twi->TWI_RHR;
	}
	p_twi->TWI_CR = 1<<1;    // send stop condition
	while ((p_twi->TWI_SR & (1<<1)) == 0){} // poll RXRDY
	data[length-1] = p_twi->TWI_RHR;
	while ((p_twi->TWI_SR & (1<<0)) == 0){} // poll TXCOMP
	return 0;
}



