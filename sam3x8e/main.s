	.syntax unified
	.cpu cortex-m3
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 6
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.thumb
	.syntax unified
	.file	"main.c"
	.section	.rodata
	.align	2
	.type	clock_pin_c, %object
	.size	clock_pin_c, 4
clock_pin_c:
	.word	268435456
	.align	2
	.type	data_pin_c, %object
	.size	data_pin_c, 4
data_pin_c:
	.word	67108864
	.text
	.align	2
	.global	main
	.thumb
	.thumb_func
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{r7, lr}
	sub	sp, sp, #16
	add	r7, sp, #0
	ldr	r0, .L2
	bl	scb_enable_handlers
	ldr	r0, .L2+4
	bl	rtt_init
	movs	r1, #0
	ldr	r0, .L2+8
	bl	wdt_enable
	ldr	r0, .L2+12
	bl	pmc_24MHz
	bl	apdu_init
	movs	r0, #2
	bl	tc_get_channel
	str	r0, [r7, #12]
	movs	r0, #5
	bl	tc_get_channel
	str	r0, [r7, #8]
	movs	r0, #7
	bl	tc_get_channel
	str	r0, [r7, #4]
	movs	r2, #1
	mov	r1, #500
	ldr	r0, [r7, #12]
	bl	tc_init_capture_mode
	movs	r2, #1
	mov	r1, #500
	ldr	r0, [r7, #8]
	bl	tc_init_capture_mode
	movs	r2, #1
	mov	r1, #500
	ldr	r0, [r7, #4]
	bl	tc_init_capture_mode
	movs	r3, #1
	movs	r2, #0
	movs	r1, #100
	ldr	r0, .L2+16
	bl	systick_enable
	mov	r1, #134217728
	ldr	r0, .L2+20
	bl	pio_select_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L2+24
	bl	pio_select_output_pins
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L2+24
	bl	pio_select_output_pins
	mov	r1, #536870912
	ldr	r0, .L2+24
	bl	pio_select_peripheral_B
	movs	r1, #128
	ldr	r0, .L2+28
	bl	pio_select_peripheral_B
	ldr	r0, .L2+32
	bl	apdu_loop
	nop
	adds	r7, r7, #16
	mov	sp, r7
	@ sp needed
	pop	{r7, pc}
.L3:
	.align	2
.L2:
	.word	-536810240
	.word	1074666032
	.word	1074666064
	.word	1074660864
	.word	-536813552
	.word	1074663424
	.word	1074663936
	.word	1074664448
	.word	handle_apdu
	.size	main, .-main
	.align	2
	.global	swd_read
	.thumb
	.thumb_func
	.type	swd_read, %function
swd_read:
	@ args = 0, pretend = 0, frame = 32
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{r4, r7, lr}
	sub	sp, sp, #36
	add	r7, sp, #0
	mov	r3, r0
	str	r1, [r7]
	strb	r3, [r7, #7]
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_select_output_pins
	movs	r3, #7
	str	r3, [r7, #28]
	b	.L5
.L8:
	ldrb	r2, [r7, #7]	@ zero_extendqisi2
	ldr	r3, [r7, #28]
	asr	r3, r2, r3
	and	r3, r3, #1
	cmp	r3, #0
	beq	.L6
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_set_output_pins
	b	.L7
.L6:
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_clear_output_pins
.L7:
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_clear_output_pins
	ldr	r3, [r7, #28]
	subs	r3, r3, #1
	str	r3, [r7, #28]
.L5:
	ldr	r3, [r7, #28]
	cmp	r3, #0
	bge	.L8
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_clear_output_pins
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_select_input_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_clear_output_pins
	movs	r3, #0
	str	r3, [r7, #24]
	movs	r3, #0
	str	r3, [r7, #20]
	b	.L9
.L11:
	ldr	r0, .L21
	bl	pio_read_input_pins
	mov	r2, r0
	mov	r3, #67108864
	ands	r3, r3, r2
	cmp	r3, #0
	beq	.L10
	movs	r2, #1
	ldr	r3, [r7, #20]
	lsl	r3, r2, r3
	mov	r2, r3
	ldr	r3, [r7, #24]
	orrs	r3, r3, r2
	str	r3, [r7, #24]
.L10:
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_clear_output_pins
	ldr	r3, [r7, #20]
	adds	r3, r3, #1
	str	r3, [r7, #20]
.L9:
	ldr	r3, [r7, #20]
	cmp	r3, #2
	ble	.L11
	ldr	r3, [r7, #24]
	cmp	r3, #2
	beq	.L13
	cmp	r3, #4
	beq	.L14
	cmp	r3, #1
	nop
	movs	r3, #0
	str	r3, [r7, #16]
	ldr	r3, [r7]
	movs	r2, #0
	str	r2, [r3]
	movs	r3, #0
	str	r3, [r7, #12]
	b	.L17
.L13:
	movs	r3, #3
	b	.L16
.L14:
	movs	r3, #1
	b	.L16
.L19:
	ldr	r0, .L21
	bl	pio_read_input_pins
	mov	r2, r0
	mov	r3, #67108864
	ands	r3, r3, r2
	cmp	r3, #0
	beq	.L18
	ldr	r3, [r7, #16]
	cmp	r3, #0
	ite	eq
	moveq	r3, #1
	movne	r3, #0
	uxtb	r3, r3
	str	r3, [r7, #16]
	ldr	r3, [r7]
	ldr	r3, [r3]
	movs	r1, #1
	ldr	r2, [r7, #12]
	lsl	r2, r1, r2
	orrs	r2, r2, r3
	ldr	r3, [r7]
	str	r2, [r3]
.L18:
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_clear_output_pins
	ldr	r3, [r7, #12]
	adds	r3, r3, #1
	str	r3, [r7, #12]
.L17:
	ldr	r3, [r7, #12]
	cmp	r3, #31
	ble	.L19
	ldr	r3, [r7, #16]
	cmp	r3, #0
	ite	ne
	movne	r3, #1
	moveq	r3, #0
	uxtb	r4, r3
	ldr	r0, .L21
	bl	pio_read_input_pins
	mov	r2, r0
	mov	r3, #67108864
	ands	r3, r3, r2
	cmp	r3, #0
	ite	eq
	moveq	r3, #1
	movne	r3, #0
	uxtb	r3, r3
	eors	r3, r3, r4
	uxtb	r3, r3
	str	r3, [r7, #8]
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_clear_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L21
	bl	pio_clear_output_pins
	ldr	r3, [r7, #8]
	cmp	r3, #0
	beq	.L20
	movs	r3, #0
	b	.L16
.L20:
	movs	r3, #2
.L16:
	mov	r0, r3
	adds	r7, r7, #36
	mov	sp, r7
	@ sp needed
	pop	{r4, r7, pc}
.L22:
	.align	2
.L21:
	.word	1074663936
	.size	swd_read, .-swd_read
	.align	2
	.global	swd_write
	.thumb
	.thumb_func
	.type	swd_write, %function
swd_write:
	@ args = 0, pretend = 0, frame = 32
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{r7, lr}
	sub	sp, sp, #32
	add	r7, sp, #0
	mov	r3, r0
	str	r1, [r7]
	strb	r3, [r7, #7]
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_select_output_pins
	movs	r3, #7
	str	r3, [r7, #28]
	b	.L24
.L27:
	ldrb	r2, [r7, #7]	@ zero_extendqisi2
	ldr	r3, [r7, #28]
	asr	r3, r2, r3
	and	r3, r3, #1
	cmp	r3, #0
	beq	.L25
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_set_output_pins
	b	.L26
.L25:
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_clear_output_pins
.L26:
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_clear_output_pins
	ldr	r3, [r7, #28]
	subs	r3, r3, #1
	str	r3, [r7, #28]
.L24:
	ldr	r3, [r7, #28]
	cmp	r3, #0
	bge	.L27
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_select_input_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_clear_output_pins
	movs	r3, #0
	str	r3, [r7, #24]
	movs	r3, #0
	str	r3, [r7, #20]
	b	.L28
.L30:
	ldr	r0, .L42
	bl	pio_read_input_pins
	mov	r2, r0
	mov	r3, #67108864
	ands	r3, r3, r2
	cmp	r3, #0
	beq	.L29
	movs	r2, #1
	ldr	r3, [r7, #20]
	lsl	r3, r2, r3
	mov	r2, r3
	ldr	r3, [r7, #24]
	orrs	r3, r3, r2
	str	r3, [r7, #24]
.L29:
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_clear_output_pins
	ldr	r3, [r7, #20]
	adds	r3, r3, #1
	str	r3, [r7, #20]
.L28:
	ldr	r3, [r7, #20]
	cmp	r3, #2
	ble	.L30
	ldr	r3, [r7, #24]
	cmp	r3, #2
	beq	.L32
	cmp	r3, #4
	beq	.L33
	cmp	r3, #1
	nop
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_clear_output_pins
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_select_output_pins
	movs	r3, #0
	str	r3, [r7, #16]
	movs	r3, #0
	str	r3, [r7, #12]
	b	.L36
.L32:
	movs	r3, #3
	b	.L35
.L33:
	movs	r3, #1
	b	.L35
.L39:
	movs	r2, #1
	ldr	r3, [r7, #12]
	lsl	r3, r2, r3
	mov	r2, r3
	ldr	r3, [r7]
	ands	r3, r3, r2
	cmp	r3, #0
	beq	.L37
	ldr	r3, [r7, #16]
	cmp	r3, #0
	ite	eq
	moveq	r3, #1
	movne	r3, #0
	uxtb	r3, r3
	str	r3, [r7, #16]
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_set_output_pins
	b	.L38
.L37:
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_clear_output_pins
.L38:
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_clear_output_pins
	ldr	r3, [r7, #12]
	adds	r3, r3, #1
	str	r3, [r7, #12]
.L36:
	ldr	r3, [r7, #12]
	cmp	r3, #31
	ble	.L39
	ldr	r3, [r7, #16]
	cmp	r3, #0
	beq	.L40
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_set_output_pins
	b	.L41
.L40:
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_clear_output_pins
.L41:
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_clear_output_pins
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_clear_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L42
	bl	pio_clear_output_pins
	movs	r3, #0
.L35:
	mov	r0, r3
	adds	r7, r7, #32
	mov	sp, r7
	@ sp needed
	pop	{r7, pc}
.L43:
	.align	2
.L42:
	.word	1074663936
	.size	swd_write, .-swd_write
	.align	2
	.global	handle_apdu
	.thumb
	.thumb_func
	.type	handle_apdu, %function
handle_apdu:
	@ args = 0, pretend = 0, frame = 168
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{r7, lr}
	sub	sp, sp, #168
	add	r7, sp, #0
	ldr	r3, .L95
	str	r3, [r7, #136]
	mov	r3, #36864
	strh	r3, [r7, #164]	@ movhi
	ldr	r3, .L95+4
	ldrh	r3, [r3, #2]
	cmp	r3, #260
	bls	.L45
	mov	r3, #26368
	strh	r3, [r7, #164]	@ movhi
	b	.L46
.L45:
	ldr	r3, .L95+4
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #66
	bne	.L47
	mov	r1, #300
	movs	r0, #5
	bl	utils_blink
	b	.L46
.L47:
	ldr	r3, .L95+4
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #68
	bne	.L48
	ldr	r3, .L95+8
	ldr	r3, [r3]
	str	r3, [r7, #132]
	ldr	r3, [r7, #132]
	str	r3, [r7, #128]
	ldr	r3, .L95+12
	str	r3, [r7, #124]
	movs	r3, #0
	strh	r3, [r7, #166]	@ movhi
	b	.L49
.L50:
	ldrh	r3, [r7, #166]
	ldr	r2, [r7, #128]
	add	r3, r3, r2
	ldrh	r2, [r7, #166]
	ldr	r1, [r7, #124]
	add	r2, r2, r1
	ldrb	r2, [r2]	@ zero_extendqisi2
	strb	r2, [r3]
	ldrh	r3, [r7, #166]
	adds	r3, r3, #1
	strh	r3, [r7, #166]	@ movhi
.L49:
	ldrh	r2, [r7, #166]
	ldr	r3, .L95+4
	ldrh	r3, [r3, #2]
	subs	r3, r3, #4
	cmp	r2, r3
	blt	.L50
	b	.L46
.L48:
	ldr	r3, .L95+4
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #69
	bne	.L51
	movs	r3, #0
	str	r3, [r7, #160]
	b	.L52
.L53:
	ldr	r2, .L95+4
	ldr	r3, [r7, #160]
	add	r3, r3, r2
	adds	r3, r3, #4
	ldrb	r3, [r3]	@ zero_extendqisi2
	mov	r1, r3
	ldr	r0, [r7, #136]
	bl	uart_send_uint8
	ldr	r3, [r7, #160]
	adds	r3, r3, #1
	str	r3, [r7, #160]
.L52:
	ldr	r3, .L95+4
	ldrh	r3, [r3, #264]
	mov	r2, r3
	ldr	r3, [r7, #160]
	cmp	r2, r3
	bgt	.L53
	b	.L46
.L51:
	ldr	r3, .L95+4
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #70
	bne	.L54
	ldr	r3, .L95+8
	ldrh	r3, [r3]
	cmp	r3, #1
	bne	.L55
	ldr	r3, .L95+16
	str	r3, [r7, #156]
	b	.L56
.L55:
	ldr	r3, .L95+20
	str	r3, [r7, #156]
.L56:
	ldr	r3, .L95+24
	ldrh	r3, [r3]
	mov	r1, r3
	ldr	r0, [r7, #156]
	bl	eefc_erase_and_write_page
	mov	r3, r0
	strh	r3, [r7, #122]	@ movhi
	ldrh	r3, [r7, #122]
	mov	r1, r3
	ldr	r0, [r7, #136]
	bl	uart_send_uint16
	b	.L46
.L54:
	ldr	r3, .L95+4
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #71
	bne	.L57
	ldr	r3, .L95+8
	ldr	r3, [r3]
	str	r3, [r7, #116]
	ldr	r3, [r7, #116]
	orr	r3, r3, #1
	str	r3, [r7, #116]
	ldr	r3, [r7, #116]
	str	r3, [r7, #112]
	ldr	r3, [r7, #112]
	blx	r3
	b	.L46
.L57:
	ldr	r3, .L95+4
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #73
	bne	.L58
	ldr	r3, .L95+4
	ldrh	r3, [r3, #2]
	and	r3, r3, #3
	uxth	r3, r3
	cmp	r3, #0
	beq	.L59
	movw	r3, #26369
	strh	r3, [r7, #164]	@ movhi
	b	.L46
.L59:
	ldr	r3, .L95+8
	ldr	r3, [r3]
	str	r3, [r7, #108]
	ldr	r3, [r7, #108]
	str	r3, [r7, #104]
	ldr	r3, .L95+12
	str	r3, [r7, #100]
	movs	r3, #0
	strh	r3, [r7, #166]	@ movhi
	b	.L61
.L62:
	ldrh	r3, [r7, #166]
	lsls	r3, r3, #2
	ldr	r2, [r7, #104]
	add	r3, r3, r2
	ldrh	r2, [r7, #166]
	lsls	r2, r2, #2
	ldr	r1, [r7, #100]
	add	r2, r2, r1
	ldr	r2, [r2]
	str	r2, [r3]
	ldrh	r3, [r7, #166]
	adds	r3, r3, #1
	strh	r3, [r7, #166]	@ movhi
.L61:
	ldrh	r2, [r7, #166]
	ldr	r3, .L95+4
	ldrh	r3, [r3, #2]
	lsrs	r3, r3, #2
	uxth	r3, r3
	subs	r3, r3, #1
	cmp	r2, r3
	blt	.L62
	b	.L46
.L58:
	ldr	r3, .L95+4
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #79
	bne	.L63
	ldr	r3, .L95+4
	ldrh	r3, [r3, #2]
	and	r3, r3, #3
	uxth	r3, r3
	cmp	r3, #0
	beq	.L64
	movw	r3, #26370
	strh	r3, [r7, #164]	@ movhi
	b	.L46
.L64:
	ldr	r3, .L95+8
	ldr	r3, [r3]
	str	r3, [r7, #96]
	ldr	r3, [r7, #96]
	str	r3, [r7, #92]
	ldr	r3, .L95+12
	str	r3, [r7, #88]
	movs	r3, #0
	strh	r3, [r7, #166]	@ movhi
	b	.L66
.L67:
	ldrh	r3, [r7, #166]
	lsls	r3, r3, #2
	ldr	r2, [r7, #92]
	add	r3, r3, r2
	ldrh	r2, [r7, #166]
	lsls	r2, r2, #2
	ldr	r1, [r7, #92]
	add	r2, r2, r1
	ldr	r1, [r2]
	ldrh	r2, [r7, #166]
	lsls	r2, r2, #2
	ldr	r0, [r7, #88]
	add	r2, r2, r0
	ldr	r2, [r2]
	orrs	r2, r2, r1
	str	r2, [r3]
	ldrh	r3, [r7, #166]	@ movhi
	adds	r3, r3, #4
	strh	r3, [r7, #166]	@ movhi
.L66:
	ldrh	r2, [r7, #166]
	ldr	r3, .L95+4
	ldrh	r3, [r3, #2]
	subs	r3, r3, #4
	cmp	r2, r3
	blt	.L67
	b	.L46
.L63:
	ldr	r3, .L95+4
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #80
	bne	.L68
	ldr	r3, .L95+4
	ldrb	r3, [r3, #4]	@ zero_extendqisi2
	cmp	r3, #255
	bne	.L69
	movs	r0, #1
	bl	eefc_clear_gpnvm_bit
	b	.L70
.L69:
	ldr	r3, .L95+4
	ldrb	r3, [r3, #4]	@ zero_extendqisi2
	cmp	r3, #0
	bne	.L71
	movs	r0, #1
	bl	eefc_set_gpnvm_bit
	b	.L70
.L71:
	ldr	r3, .L95+4
	ldrb	r3, [r3, #4]	@ zero_extendqisi2
	cmp	r3, #1
	bne	.L72
	movs	r0, #1
	bl	eefc_set_gpnvm_bit
	movs	r0, #2
	bl	eefc_set_gpnvm_bit
	b	.L70
.L72:
	mov	r3, #27264
	strh	r3, [r7, #164]	@ movhi
.L70:
	add	r3, r7, #43
	mov	r0, r3
	bl	eefc_get_gpnvm
	ldrb	r3, [r7, #43]	@ zero_extendqisi2
	mov	r1, r3
	ldr	r0, [r7, #136]
	bl	uart_send_uint8
	b	.L46
.L68:
	ldr	r3, .L95+4
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #82
	bne	.L73
	ldr	r3, .L95+8
	ldr	r3, [r3]
	str	r3, [r7, #84]
	ldr	r3, [r7, #84]
	str	r3, [r7, #80]
	movs	r3, #0
	strh	r3, [r7, #166]	@ movhi
	b	.L74
.L75:
	ldrh	r3, [r7, #166]
	ldr	r2, [r7, #80]
	add	r3, r3, r2
	ldrb	r3, [r3]	@ zero_extendqisi2
	mov	r1, r3
	ldr	r0, [r7, #136]
	bl	uart_send_uint8
	ldrh	r3, [r7, #166]
	adds	r3, r3, #1
	strh	r3, [r7, #166]	@ movhi
.L74:
	ldr	r3, .L95+4
	ldrh	r3, [r3, #264]
	ldrh	r2, [r7, #166]
	cmp	r2, r3
	bcc	.L75
	b	.L46
.L73:
	ldr	r3, .L95+4
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #83
	bne	.L76
	movs	r0, #1
	bl	rstc_reset
	b	.L46
.L96:
	.align	2
.L95:
	.word	1074661376
	.word	apdu
	.word	apdu+4
	.word	apdu+8
	.word	1074662400
	.word	1074661888
	.word	apdu+6
.L76:
	ldr	r3, .L97
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #84
	bne	.L77
	ldr	r3, .L97
	ldrh	r3, [r3, #264]
	mov	r2, r3
	ldr	r1, .L97+4
	ldr	r0, [r7, #136]
	bl	uart_pdc_transfer
	b	.L46
.L77:
	ldr	r3, .L97
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #87
	bne	.L78
	ldr	r3, .L97+8
	str	r3, [r7, #76]
	ldr	r1, [r7, #76]
	ldr	r0, [r7, #136]
	bl	uart_send_uint32
	b	.L46
.L78:
	ldr	r3, .L97
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #76
	bne	.L79
	ldr	r3, .L97+4
	ldr	r3, [r3]
	str	r3, [r7, #72]
	ldr	r3, .L97+12
	ldr	r3, [r3]
	str	r3, [r7, #68]
	ldr	r1, [r7, #72]
	mov	r3, r7
	ldr	r2, [r7, #68]
	mov	r0, r3
	bl	blake2s
	movs	r3, #0
	str	r3, [r7, #152]
	b	.L80
.L81:
	mov	r2, r7
	ldr	r3, [r7, #152]
	add	r3, r3, r2
	ldrb	r3, [r3]	@ zero_extendqisi2
	mov	r1, r3
	ldr	r0, .L97+16
	bl	uart_send_uint8
	ldr	r3, [r7, #152]
	adds	r3, r3, #1
	str	r3, [r7, #152]
.L80:
	ldr	r3, [r7, #152]
	cmp	r3, #31
	ble	.L81
	b	.L46
.L79:
	ldr	r3, .L97
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #89
	bne	.L82
	ldr	r0, .L97+20
	bl	systick_get_value
	mov	r3, r0
	mov	r1, r3
	ldr	r0, [r7, #136]
	bl	uart_send_uint32
	b	.L46
.L82:
	ldr	r3, .L97
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #67
	bne	.L83
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_select_output_pins
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_set_output_pins
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_set_output_pins
	movs	r3, #0
	str	r3, [r7, #148]
	b	.L84
.L85:
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_clear_output_pins
	ldr	r3, [r7, #148]
	adds	r3, r3, #1
	str	r3, [r7, #148]
.L84:
	ldr	r3, [r7, #148]
	cmp	r3, #59
	ble	.L85
	movs	r3, #15
	str	r3, [r7, #144]
	b	.L86
.L89:
	movw	r2, #31207
	ldr	r3, [r7, #144]
	asr	r3, r2, r3
	and	r3, r3, #1
	cmp	r3, #0
	beq	.L87
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_set_output_pins
	b	.L88
.L87:
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_clear_output_pins
.L88:
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_clear_output_pins
	ldr	r3, [r7, #144]
	subs	r3, r3, #1
	str	r3, [r7, #144]
.L86:
	ldr	r3, [r7, #144]
	cmp	r3, #0
	bge	.L89
	mov	r3, #67108864
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_clear_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_clear_output_pins
	movs	r3, #0
	str	r3, [r7, #140]
	b	.L90
.L91:
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_set_output_pins
	mov	r3, #268435456
	mov	r1, r3
	ldr	r0, .L97+24
	bl	pio_clear_output_pins
	ldr	r3, [r7, #140]
	adds	r3, r3, #1
	str	r3, [r7, #140]
.L90:
	ldr	r3, [r7, #140]
	cmp	r3, #19
	ble	.L91
	movs	r3, #0
	str	r3, [r7, #36]
	add	r3, r7, #36
	mov	r1, r3
	movs	r0, #165
	bl	swd_read
	str	r0, [r7, #64]
	ldr	r3, [r7, #36]
	mov	r1, r3
	ldr	r0, .L97+16
	bl	uart_send_uint32
	ldr	r3, [r7, #64]
	strh	r3, [r7, #164]	@ movhi
	b	.L46
.L83:
	ldr	r3, .L97
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #48
	bne	.L92
	ldr	r3, .L97
	ldrb	r3, [r3, #4]
	strb	r3, [r7, #63]
	movs	r3, #0
	str	r3, [r7, #32]
	add	r2, r7, #32
	ldrb	r3, [r7, #63]	@ zero_extendqisi2
	mov	r1, r2
	mov	r0, r3
	bl	swd_read
	str	r0, [r7, #56]
	ldr	r3, [r7, #32]
	mov	r1, r3
	ldr	r0, .L97+16
	bl	uart_send_uint32
	ldr	r3, [r7, #56]
	strh	r3, [r7, #164]	@ movhi
	b	.L46
.L92:
	ldr	r3, .L97
	ldrb	r3, [r3]	@ zero_extendqisi2
	cmp	r3, #49
	bne	.L93
	ldr	r3, .L97+4
	ldr	r3, [r3]
	str	r3, [r7, #52]
	ldr	r3, .L97
	ldrb	r3, [r3, #8]
	strb	r3, [r7, #51]
	ldrb	r3, [r7, #51]	@ zero_extendqisi2
	ldr	r1, [r7, #52]
	mov	r0, r3
	bl	swd_write
	str	r0, [r7, #44]
	ldr	r3, [r7, #44]
	strh	r3, [r7, #164]	@ movhi
	b	.L46
.L93:
	mov	r3, #27904
	strh	r3, [r7, #164]	@ movhi
.L46:
	ldrh	r3, [r7, #164]
	mov	r1, r3
	ldr	r0, [r7, #136]
	bl	uart_send_uint16
	mov	r3, r0
	mov	r0, r3
	adds	r7, r7, #168
	mov	sp, r7
	@ sp needed
	pop	{r7, pc}
.L98:
	.align	2
.L97:
	.word	apdu
	.word	apdu+4
	.word	uart_send_uint8
	.word	apdu+8
	.word	1074661376
	.word	-536813552
	.word	1074663936
	.size	handle_apdu, .-handle_apdu
	.ident	"GCC: (15:5.4.1+svn241155-1) 5.4.1 20160919"
