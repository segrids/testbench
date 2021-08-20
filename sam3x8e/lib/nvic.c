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
 
#include "nvic.h"

/*
 *
 * NVIC = Nested Vectored Interrupt Controller
 * 
 * Exception number = vector table index; three types of exceptions: 1:Reset,
 *                    2:NMI, 3-7: Faults, 12-16: System exceptions, 16-45: Interrupts,
 *                    NVIC only controlls Interrupts
 * 
 * ISR_Number = Field in the core register "Interrupt Program Status Register",
 *              indicates, if core is runing in thread mode or one of the exceptions
 *              In the second case, the ISR_Number is the exception number
 * 
 * PID = Numbering of peripherals; a peripheral corresponds to one ISR
 * 
 * IRQ = similar to PID, but negative numers used for Faults and System exceptions
 *       in CMSIS
 * 
 * Ex.Num |                          | PID | IRQ  | ISR_Number 
 * ------ | ------------------------ | --- | ---- | -------------- 
 * 0      | Initial Stack Pointer    |     |      | 0=Thread Mode 
 * 1      | Reset                    |     | -14  | 1=Reserved 
 * 2      | Reserved                 |     | -13  | 2=NMI 
 * 3      | Hard Fault               |     | -12  | 3=Hard Fault 
 * ...    | ...                      |     | ...  | ... 
 * 15     | Systick                  |     | -1   | 15=SysTick 
 * 16     | Supply Controller (SUPC) |  0  |  0   | 16=SUPC 
 * 17     | Reset Controller (RSTC)  |  1  |  1   | 17=RSTC 
 * ...    | ...                      | ... | ...  | ... 
 * 60     | CAN1                     | 44  |  44  | 60=CAN1 
 * 
 * 
 */

/* nvic_enable() 
 *
 * enable interrupt for PID 
 **/
void nvic_enable(Nvic* p_nvic, int pid) {
	int instance = (pid / 30) & 0x1;
	pid = pid - instance*29;
	p_nvic->ISER[instance] = 1 << pid;
}

/* nvic_disable() */
void nvic_disable(Nvic* p_nvic, int pid) {
	int instance = (pid / 30) & 0x1;
	pid = pid - instance*29;
	p_nvic->ICER[instance] = 1 << pid;
}

/* nvic_set_pending() */
void nvic_set_pending(Nvic* p_nvic, int pid) {
	int instance = (pid / 30) & 0x1;
	pid = pid - instance*29;
	p_nvic->ISPR[instance] = 1 << pid;
}

/* nvic_clear_pending() */
void nvic_clear_pending(Nvic* p_nvic, int pid) {
	int instance = (pid / 30) & 0x1;
	pid = pid - instance*29;
	p_nvic->ICPR[instance] = 1 << pid;
}

/* nvic_is_active() */
int nvic_is_active(Nvic* p_nvic, int pid) {
	int instance = (pid / 30) & 0x1;
	pid = pid - instance*29;
	return (p_nvic->IABR[instance] >> pid) & 0x1;
}

/* nvic_set_priority()
 *
 * change interrupt priority for a pid <30.
 */
void nvic_set_priority(Nvic* p_nvic, int pid, int prio) {
	p_nvic->IP[pid & 0x1f] = prio & 0x0f;
}

