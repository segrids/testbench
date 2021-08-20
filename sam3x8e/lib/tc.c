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

#include "tc.h"
#include "nvic.h"
#include "pmc.h"
#include "pio.h"

/*
 * Timer Counter, see Chapter 36
 * 
 * -> 3 Timer Counter Modules
 * -> each module has 3 identical Channels => 9 channels TC0 .... TC8 (PID 27....PID35)
 * -> each channel has three external clock inputs, five internal clock inputs and two multi-purpose input/output signals
 * -> each channel drives an internal interrupt signal which can be programmed togenerate processor interrupts.
 * 
 *  Global registers which act upon all TC channels:
 * - Block Control Register (TC_BCR)—allows channels to be started simultaneously with the same instruction
 * - Block Mode Register (TC_BMR)—defines the external clock inputs for each channel, allowing them to be chained
 * 
 * TIMER_CLOCK1 	MCK/2
 * TIMER_CLOCK2 	MCK/8
 * TIMER_CLOCK3 	MCK/32
 * TIMER_CLOCK4 	MCK/128
 * TIMER_CLOCK5 	SLCK
 * 
 * capture mode: act as a stopwatch; triggers by TIOA / TIOB
 * waveform mode: act as a signal generator; output signal at TIOA; TIOB is either a second output, or a trigger input
 * 
 * In waveform mode, the following events control TIOA and TIOB: software trigger, external event and RC compare. 
 * RA compare controls TIOA and RB compare controls TIOB. Each of these events can be programmed to set, clear 
 * or toggle the output as defined in the corresponding parameter in TC_CMR.
 * 
 * SYNC is a signal for timer synchronization
 * 
 * --------- NVIC[0]
 * TC0 27
 * TC1 28
 * TC2 29
 * ---------- NVIC[1] 
 * TC3 30
 * TC4 31
 * TC5 32
 * TC6 33
 * TC7 34
 * TC8 35
 * 
*/

/* tc_pmc_enable() 
 *
 * Enable clock for a TC channel.
 */
void tc_pmc_enable(uint32_t channel){
	pmc_enable_peripheral_clock(PMC, 27+channel);
}

/* tc_pmc_disable() 
 *
 * Disable clock for a TC channel.
 */
void tc_pmc_disable(uint32_t channel){
	pmc_disable_peripheral_clock(PMC, 27+channel);
}

/* tc_nvic_enable() 
 *
 * Enable interrupts for a TC channel.
 */
void tc_nvic_enable(uint32_t channel){
	nvic_enable(NVIC, 27+channel);
}

/* tc_nvic_disable() 
 *
 * Disable interrupts for a TC channel.
 */
void tc_nvic_disable(uint32_t channel){
	nvic_disable(NVIC, 27+channel);
}

/* tc_deassert_irq()
 *
 * Read TC status register de-asserts interrupt request 
 * Required to call this function inside interrupt routine.
 * Otherwise endless loop of ISR.
 */
uint32_t tc_deassert_irq(TcChannel* pTc){
	return pTc->TC_SR;
}


/* tc_enable_interrupt()
 *
 * Enable a specific interrupt for a TC channel indicated by a number:
 * 
 * Number    | Mnemonic  | Name              | Desciption
 * --------- | --------- | ----------------- | -------------------------------------------------------- 
 *         0 |     COVFS | Counter Overflow  | value overflows 0xffffffff 
 *         1 |     LOVRS | Load Overrun      | ? 
 *         2 |     CPAS  | RA Compare        | value = Register A, toggle TIOA in waveform mode 
 *         3 |     CPBS  | RB Compare        | value = Register B, toggle TIOB in waveform mode 
 *         4 |     CPCS  | RC Compare        | value = Register C, value reset to zero in both modes 
 *         5 |     LDRAS | RA Loading        | capture mode trigger event on TIOA 
 *         6 |     LDRBS | RB Loading        | capture mode trigger event on TIOB 
 *         7 |     ETRGS | External Trigger  | 
 * 
*/
void tc_enable_interrupt(TcChannel * pTc, uint32_t number){
	pTc->TC_IER = 1 << number;
}

/* tc_disable_interrupt()
 *
 * disable a specific interrupt for a TC channel indicated by a number:
 */
void tc_disable_interrupt(TcChannel * pTc, uint32_t number){
	pTc->TC_IDR = 1 << number;
}

/* tc_reset 
 *
 * Reset a TC channel
 */
void tc_reset(TcChannel * pTc){
	pTc->TC_CCR = 5;
}

/* tc_get_value()
 *
 * Read a Timer/Counter value.
 */
uint32_t tc_get_value(TcChannel * pTc){
	return pTc->TC_CV;
}


/* tc_init_capture_mode()
 *
 * Initialize a TC channel in capture mode.
 *
 * Args:
 * c_value: Value of register C.
 * clock_select: select clock  according to:
 *
 * clock_select	| TC freq
 *  ------------------------
 *           0   | MCK /2
 *           1   | MCK /8
 *           2   | MCK /32
 *           3   | MCK /128
 *           
 */
void tc_init_capture_mode(TcChannel * pTc, uint32_t c_value, uint32_t clock_select){
	// 1 TC Channel Control Register (enable, disable, SW trigger)
	//   -> TC_CCR
	// 2 TC Channel Mode Register: (prescaler, trigger edge, ext trigger A or B, waveform or capture mode)
	//   -> TC_CMR
	//	bits 0,1 = prescaler (to be modified in tests), initial value = 0
	//	bit 10 = ext Trigger A(1) , or B(0), initial value = 0
	//	bit 14 = clear Register C at compare trigger
	//	bit 15 = waveform (1) or capture mode, initial value = 0
	pTc->TC_CMR = (clock_select & 0x3);
	//*tc_cmr0 = 0x0019c000;
	// 8 TC Register C
	//   -> TC_RC
	//	to be modified in tests
	pTc->TC_RC = c_value;
	// 10 TC Interrupt Enable Register
	//   -> TC_IER
	//	to be modified in tests
	// 11 TC Interrupt Disable Register
	//    -> TC_IDR
	//	to be modified in tests
	// 12 TC Interrupt Mask Register
	//    -> TC_IMR
	//	to be modified in tests
	// 13 TC Block Control Register
	//	to be modified in tests
	// 1 TC Channel Control Register (enable, disable, SW trigger)
	//	bit 0 = enable TC0
	//	bit 2 = SW trigger to start the timer
	pTc->TC_CCR = 5;
}

/* tc_init_waveform_mode()
 *
 * Choose standard configuration for TC channel.
 */
void tc_init_waveform_mode(TcChannel * pTc){
	// TC0 CONFIGURATION, Chapter 36, PID27
	// WAVSEL = 10: TIOA Waveform to be configured by RA and RC
	// WAVE = 1 : Waveform Mode enabled
	// ACPA = 1 : RA Compare Effect on TIOA = Set
	// ACPC = 2 : RC Compare Effect on TIOA = Clear
	// ASWTRG = 1: Software Trigger Effect on TIOA = Set
	pTc->TC_CMR = 0x0019c000;
	//*tc_cmr0 = 0x0019c000;
	pTc->TC_RA = 300;
	//*tc_ra0 = 300;
	pTc->TC_RC = 500;
	//*tc_rc0 = 500; // we have MCK/1000 on the output pin
	pTc->TC_CCR = 1 << 0;
	//*tc_ccr0 = 1 << 0; //enable TC0 (write-only register)
	pTc->TC_CCR = 1 << 2;
	//*tc_ccr0 = 1 << 2;	//SW Trigger setting SWTRG in TC_CCR
}

