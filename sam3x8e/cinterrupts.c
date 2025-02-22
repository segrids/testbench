
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
*/



#include "startup.h"
#include "types.h"
#include "scb.h" 
#include "utils.h"
#include "slave.h"
#include "apdu.h"


/*
A non maskable interrupt (NMI) can be signalled by a peripheral or triggered by software. This is the highest
priority exception other than reset. It is permanently enabled and has a fixed priority of -2.
NMIs cannot be:
- Masked or prevented from activation by any other exception.
- Preempted by any exception other than Reset.
*/
void NMI_Handler(void) {
	utils_error(0x7);
}

/*
A hard fault is an exception that occurs because of an error during exception processing, or because an exception
cannot be managed by any other exception mechanism. Hard faults have a fixed priority of -1, meaning they have
higher priority than any exception with configurable priority.
*/
void HardFault_Handler(void) {
	utils_error(0x3);
}

/*
A memory management fault is an exception that occurs because of a memory protection related fault. The MPU
or the fixed memory protection constraints determines this fault, for both instruction and data memory transactions.
This fault is used to abort instruction accesses to Execute Never (XN) memory regions, even if the MPU is
disabled.
*/
void MemManage_Handler(void) {
	utils_error(0x4);
}

/*
Bus fault is an exception that occurs because of a memory related fault for an instruction or data memory
transaction. This might be from an error detected on a bus in the memory system
*/
void BusFault_Handler(void) {
	utils_error(0x5);
}


/*
usage fault is an exception that occurs because of a fault related to instruction execution. This includes:
- an undefined instruction
- an illegal unaligned access
- invalid state on instruction execution
- an error on exception return.
The following can cause a usage fault when the core is configured to report them:
- an unaligned address on word and halfword memory access
- division by zero.
*/
void UsageFault_Handler(void) {
	utils_error(0x6);
}

void SysTick_Handler(void){
	utils_toggle_led();
	for (int i=0; i<apdu.le; i++){
		slave_send_uint8(0);   // to avoid testbench hanging
	}
	slave_send_uint16(0x5451); // Error code 
	Reset_Handler();
}
