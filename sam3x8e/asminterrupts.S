#ifndef BOOTLOADER
#endif
#ifdef TESTEXCEPTIONS
/*
2019-09-18: 
	-# The Bus Fault Handler plays a special role belong the Fault Handlers: it is used for memory related tests.
	-# Implement the Bus Fault Handler and ISR in assembly!
	-# The other Fault Halndlers are implemented in c (see lib/cinterrupts.c).
	-# All ISRs and the Bus Fault Handler must only use the (stacked) registers (R0,R1,R2,R3,R12) for general purpose.
	-# In particular: no usage of R7 as Frame Pointer!
	-# All ISRs and the Bus Fault Handler must ´push {lr}´ at the beginning and ´pop {pc}´ at the end.
	-# The Bus Fault Handler must increment the stacked ReturnAddress by 2.
	-# The Bus Fault Handler shall write a return value into R4.
	-# All other Fault Handlers shall switch RED on (job for the debugger!) and remain in a loop.
*/
	.syntax unified
	.cpu cortex-m3
	.fpu softvfp
	.thumb
	.text
/*******************************************************************************************/
	.align	2
	.global	BusFault_Handler
	.thumb
	.thumb_func
	.type	BusFault_Handler, %function
/*
Bus fault is an exception that occurs because of a memory related fault for an instruction or data memory
transaction. This might be from an error detected on a bus in the memory system

Want the handler to modify the saved Return Address value on the stack.
Assume that in both modes (thread and handler) the Main stack is used.
Then the Handler finds 8 values on the stack:
PSR               [SP+28:SP+32]
PC(ReturnAddress) [SP+24:SP+28]
LR                [SP+20:SP+24]
R12               [SP+16:SP+20]
R0-R3:	          [SP:SP+16]
*/

BusFault_Handler:
	ldr r0, [sp, #24]          @ r0 = ReturnAddress
	add r0, r0, #2             @ increment by 2 in order to get back to next instruction 
	str r0, [sp, #24]          @ incremented ReturnAddress back to stack
	push	{lr}               @ lr contains EXC_RETURN, indicating if MSP or PSP to be used after return and if privileged or unprivileged code
	ldr r1, .LxE000ED24	   @ 
	ldr r4, [r1]               @ copy System Handler Control and State Register to r4
	pop	{pc}               @ load pc with EXC_RETURN which triggers the exception return (incl. restore of the 8 registers)
.LxE000ED24:
	.word	0xE000ED24         @ System Handler Control and State Register
	.size	BusFault_Handler, .-BusFault_Handler

/*******************************************************************************************/

	.align	2
	.global	TC2_Handler
	.thumb
	.thumb_func
	.type	TC2_Handler, %function
/* TC2     pid=29 */
TC2_Handler:
	@ typically, handler reads the interrupt reason from status register
	@ only the RC Compare interrupt should be enabled, where the counter value is reset
	@ change TC5 RC register to value R5 & 0xff
	@ can freely use R0-R3
	push	{r7, lr}
	nop
	@ experimental: what is the effect of FAULTMASK within an ISR ?
	nop
	nop
	nop
	@bl	utils_toggle_red
	nop
	mov R0, #1
	msr FAULTMASK, r0
	nop
	nop
	nop
	ldr r2, .L_TC2_SR
	nop
	ldr r2, [r2]
	nop
	nop 
	@ldr r2, .L_TC5_RC
	@and r0, R5,#255
	@str r0, [r2]
	pop	{r7, pc}  @ mov pc, lr would also work for cortex-m3, but not for cortex-m0
.L_TC5_RC:
	.word 1074282652 @0x4008409C
.L_TC2_SR:
	.word 1074266272 @0x400800A0
	.size TC2_Handler, .-TC2_Handler
/*******************************************************************************************/
#endif
