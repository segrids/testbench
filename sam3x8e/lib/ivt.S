/* ivt.S
 *
 * Create the Interrupt Vector Table */

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
	handler MemManage_Handler
	handler BusFault_Handler
	handler UsageFault_Handler
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	.int 0  /* Reserved */
	handler SVC_Handler
	handler DebugMon_Handler
	.int 0  /* Reserved */
	handler PendSV_Handler
	handler SysTick_Handler

	/* configurable interrupts */
	handler SUPC_Handler    /*  0  Supply Controller */
	handler RSTC_Handler    /*  1  Reset Controller */
	handler RTC_Handler     /*  2  Real Time Clock */
	handler RTT_Handler     /*  3  Real Time Timer */
	handler WDT_Handler     /*  4  Watchdog Timer */
	handler PMC_Handler     /*  5  PMC */
	handler EFC0_Handler    /*  6  EFC 0 */
	handler EFC1_Handler    /*  7  EFC 1 */
	handler UART_Handler    /*  8  UART */
	handler SMC_Handler     /*  9  SMC */
	handler SDRAMC_Handler  /* 10  SDRAMC */
	handler PIOA_Handler    /* 11 Parallel IO Controller A */
	handler PIOB_Handler    /* 12 Parallel IO Controller B */
	handler PIOC_Handler    /* 13 Parallel IO Controller C */
	handler PIOD_Handler    /* 14 Parallel IO Controller D */
	handler PIOE_Handler    /* 15 Parallel IO Controller E */
	handler PIOF_Handler    /* 16 Parallel IO Controller F */
	handler USART0_Handler  /* 17 USART 0 */
	handler USART1_Handler  /* 18 USART 1 */
	handler USART2_Handler  /* 19 USART 2 */
	handler USART3_Handler  /* 20 USART 3 */
	handler HSMCI_Handler   /* 21 MCI */
	handler TWI0_Handler    /* 22 TWI 0 */
	handler TWI1_Handler    /* 23 TWI 1 */
	handler SPI0_Handler    /* 24 SPI 0 */
	handler SPI1_Handler    /* 25 SPI 1 */
	handler SSC_Handler     /* 26 SSC */
	handler TC0_Handler     /* 27 Timer Counter 0 */
	handler TC1_Handler     /* 28 Timer Counter 1 */
	handler TC2_Handler     /* 29 Timer Counter 2 */
	handler TC3_Handler     /* 30 Timer Counter 3 */
	handler TC4_Handler     /* 31 Timer Counter 4 */
	handler TC5_Handler     /* 32 Timer Counter 5 */
	handler TC6_Handler     /* 33 Timer Counter 6 */
	handler TC7_Handler     /* 34 Timer Counter 7 */
	handler TC8_Handler     /* 35 Timer Counter 8 */
	handler PWM_Handler     /* 36 PWM */
	handler ADC_Handler     /* 37 ADC controller */
	handler DACC_Handler    /* 38 DAC controller */
	handler DMAC_Handler    /* 39 DMA Controller */
	handler UOTGHS_Handler  /* 40 USB OTG High Speed */
	handler TRNG_Handler    /* 41 True Random Number Generator */
	handler EMAC_Handler    /* 42 Ethernet MAC */
	handler CAN0_Handler    /* 43 CAN Controller 0 */
	handler CAN1_Handler     /* 44 CAN Controller 1 */
