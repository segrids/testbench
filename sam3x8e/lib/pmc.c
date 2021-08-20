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
#include "pmc.h"

/* pmc_24MHz()
 *
 * Start 24MHz Master Clock with PLLA.
 */
void pmc_24MHz(Pmc* p_pmc ) {
	// Programming Sequence (Datasheet 28.12)
	// Step 1
	// Select internal oscillator at 4MHz, Startup time 8
	p_pmc->CKGR_MOR = 0x00370809;

	// wait for external crystal oscillator to be stabilized
	while ((p_pmc->PMC_SR & 0x01) == 0) { }

	// Select external oscillator
	p_pmc->CKGR_MOR = 0x01370809;
	
	// Wait for selection to be in progress (Weird, better check)
	while (!(p_pmc->PMC_SR & (0x01 << 16))) { }

	// Step 2 (optional and skipped)

	// Select Main clock
	p_pmc->PMC_MCKR = (p_pmc->PMC_MCKR & (~(uint32_t)0x3u)) | 0x1u;
	// Wait for master clock to be ready
	while ((p_pmc->PMC_SR & (1 << 3)) == 0) { }

	// Setp 3
	p_pmc->CKGR_PLLAR = 0x20033f01;
	//*ckgr_pllar = 0x20073f01;
	// Wait for PLLA to be locked
	while ((p_pmc->PMC_SR & 0x02) == 0) { }

	// Step 4
	// PRESS and CSS should not be written in the same operation
	// be careful to only change the bits you want changed
	// Set prescaler to divide by 2
	p_pmc->PMC_MCKR = (p_pmc->PMC_MCKR & (~(uint32_t)0x70u)) | 0x11;
	while ((p_pmc->PMC_SR & 0x08) == 0) { }

	// Select PLLA Clock
	p_pmc->PMC_MCKR = (p_pmc->PMC_MCKR & (~(uint32_t)0x3u)) | 0x12;
	while ((p_pmc->PMC_SR & 0x08) == 0) { }

	// Step 5 (optional and skipped)

	// Step 6
	// p_pmc->PMC_PCER0 = 1 << 8; NOT NECCESSARY, UART is always on
	p_pmc->PMC_PCDR1 = 1 << 8; // DIABLE UOTGHS
}


/* pmc_enable_peripheral_clock()
 *
 * Clock peripheral of PID.
 */
void pmc_enable_peripheral_clock(Pmc * p_pmc, int pid){
	if (pid < 32){
		p_pmc->PMC_PCER0 = 1 << pid;
	} else {
		p_pmc->PMC_PCER1 = 1 << (pid-32);
	}
}

/* pmc_disable_peripheral_clock()
 *
 * Stop the clock of peripheral of PID.
 */
void pmc_disable_peripheral_clock(Pmc * p_pmc, int pid){
	if (pid < 32){
		p_pmc->PMC_PCDR0 = 1 << pid;
	} else {
		p_pmc->PMC_PCDR1 = 1 << (pid-32);
	}
}
