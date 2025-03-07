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
 
#include "ckcu.h"

/* CKCU at 0x40088000 */
uint32_t * const GCCR = (uint32_t *) 0x40088004; // Global Clock Control Register
uint32_t * const GCSR = (uint32_t *) 0x40088008; // Global Clock Control Register
uint32_t * const AHBCCR = (uint32_t *) 0x40088024; // AHB Clock Control Register
uint32_t * const APBCCR0 = (uint32_t *) 0x4008802c; // APB Clock Control Register 0

void ckcu_configure(void){
	*GCCR = (1 << 10) | 2; // Enable HSE and use it as CK_SYS (Highspeed External Oscillaor, 8MHz)
	while ((*GCSR & (1 << 2)) == 0){
		// poll HSE ready flag
	}
	//*AHBCCR |= 1 << 6; // APB clock enabled in sleep mode
	*AHBCCR |= 1 << 4;	 // Peripheral DMA Clock Enable
	*AHBCCR |= 1 << 13;  // CRC Clock enable
	*AHBCCR |= 1 << 16;  // GPIO Port A Clock enable; required for USART1 ???
	*AHBCCR |= 1 << 18;  // GPIO Port C Clock enable; REQUIRED to switch LED on
	*APBCCR0 |= 1 << 1;  // I2C1 clock enable
	*APBCCR0 |= 1 << 0;  // I2C0 clock enable
	*APBCCR0 |= 1 << 9;  // USART1 clock enable
	*APBCCR0 |= 1 << 14; // Alternate Function I/O Clock Enable;  not required if AF0 is used
}
