.macro handler name
	.set \name, bad_interrupt
	.weak \name
	.int \name
.endm

.section .dummy_handler
bad_interrupt:
	b bad_interrupt

.section .vectors
	.global ivt
ivt:
	.int _estack
	.int Reset_Handler
	handler NMI_Handler
	handler HardFault_Handler
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
