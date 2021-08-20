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

#include "scb.h"


/* 
 * SCB = System Control Block, see Section 10.21 of the Datasheet.
 */



/* scb_enable_handlers()
 *
 * Enable
 *	- MemManage_Handler
 *	- BusFault_Handler
 *	- UsageFault_Handler
 * 
 * Note that the following are always enabeled:
 *	- Reset_Handler
 *	- NMI_Handler
 *	- HardFault_Handler 
 */
void scb_enable_handlers(Scb* p_scb){
	p_scb->SHCRS = p_scb->SHCRS | 0x70000;
}

/* scb_move_vectors()
 *
 * Copy the 61 vectors of the vector table to a given address,
 * and let the VTOR point to this address.
 */
void scb_move_vectors(Scb* p_scb, uint32_t address){
	uint32_t * p_source = (uint32_t *) p_scb->VTOR;
	uint32_t * p_dest = (uint32_t *) address;
	for (int i = 0; i < 61; i++) {
		p_dest[i] = p_source[i];
	}
	p_scb->VTOR = address;
}

/* scb_reset_vectors()
 *
 * Select the vector table at address zero.
 */
void scb_reset_vectors(Scb* p_scb){
	p_scb->VTOR = 0;
}

/* scb_replace_vector()
 *
 * Replace single exception handler by some function,
 * required that the VTOR points to RAM.             */
void scb_replace_vector(Scb* p_scb, uint32_t exception_number, void * fct){
	uint32_t * p_vectors = (uint32_t *) p_scb->VTOR;
	p_vectors[exception_number] = (uint32_t)fct;
}


