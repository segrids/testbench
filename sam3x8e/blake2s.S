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

Author: David Kuehnemann
*/

	.syntax unified

	.text

	.global blake2s
	@ Blake2s hashing function (outlen = 32)
	@   bound before: R0, R1, R2 = output array, input address, input length
	@   bound after:
blake2s:
	@ function prologue
	push {R7, LR}
	sub SP, #64 @ 16x4=64 for v[]
	add R7, SP, #0 @ new frame pointer at top of stack

	@ copy initialization vector to h[]
	ldr R4, =iv
	mov R3, #0
loop_init:
	ldr R5, [R4, R3]
	str R5, [R0, R3]
	add R3, #4
	cmp R3, #32
	blt loop_init
	@ free R3, R4, R5

	@ h[0] ^= 0x0101000 ^ 32
	ldr R3, [R0, #0]
	mov R4, #0x01000000
	add R4, #0x00010000
	add R4, #32
	eor R3, R4
	str R3, [R0, #0]
	@ free R3, R4

	@ update, 16 words at a time
	mov R9, #64 @ t = 64
f:
	ldr R5, =iv
	mov R6, #0
loop_init_v:
	@ v[i] = h[i]
	ldr R4, [R0, R6]
	str R4, [R7, R6]
	@ v[i + 8] = iv[i]
	ldr R4, [R5, R6]
	add R6, #32
	str R4, [R7, R6]
	@ i++
	sub R6, #28
	cmp R6, #32
	blt loop_init_v
	@ free R4, R5, R6

	@ v[12] ^= t
	ldr R4, [R7, #48]
	eor R4, R9
	str R4, [R7, #48]
	@ free R4

	@ invert v[14] for last block
	@ TODO: conditional
	cmp R9, R2        @ if t >= input_len (last)
	blt invert_end
	ldr R4, [R7, #56] @ v[14]
	mvn R4, R4        @ v[14] ~= v[14]
	str R4, [R7, #56] @ save v[14]
invert_end:

	@ ten rounds of applying the mixing function
	mov R8, 0
	push {R0, R2}
	@ free R0, R2

	ldr R2, =sigma
loop_mixing:
	ldr R0, =gvec
	bl mix
	bl mix
	bl mix
	bl mix
	bl mix
	bl mix
	bl mix
	bl mix

	add R8, #1
	cmp R8, #10
	blt loop_mixing

	@ restore R0, R2
	pop {R0, R2}


	@ update h[]
	mov R3, #0
loop_update_h:
	ldr R4, [R0, R3] @ h[i]
	ldr R5, [R7, R3] @ v[i]
	eor R4, R5       @ h[i] ^= v[i]
	add R3, #32      @ i + 8
	ldr R5, [R7, R3] @ v[i + 8]
	eor R4, R5       @ h[i] ^= v[i + 8]
	sub R3, #32
	str R4, [R0, R3] @ save h[i]
	add R3, #4       @ i++

	cmp R3, #32
	blt loop_update_h

	add R9, #64 @ t += 64
	add R1, #64 @ input += 64
	cmp R9, R2  @ while t <= input_len
	ble f

	@ function epilogue
	add R7, #64
	mov SP, R7 @ return stack pointer to bottom
	pop {R7, PC} @ restore previous stack pointer and jump back


@ Mixing function:
@ 	R0, R1, R2 = gvec pointer, input pointer, sigma pointer
@ Uses:
@ 	R3, R4, R5, R6
mix:
	@ v[a] = v[a] + v[b] + x;
	ldrb R3, [R0]     @ a = gvec[0]
	ldr R4, [R7, R3]  @ v[a]
	ldrb R5, [R0, #1] @ b = gvec[1]
	ldr R5, [R7, R5]  @ v[b]
	add R4, R5        @ v[a] += v[b]
	ldrb R6, [R2], #1 @ *(sigma++)
	ldr R6, [R1, R6]  @ x
	add R4, R6        @ v[a] += x
	str R4, [R7, R3]  @ save v[a]
	@ free R3, R6

	@ v[d] = ROTR32(v[d] ^ v[a], 16);
	ldrb R3, [R0, #3] @ d = gvec[3]
	ldr R6, [R7, R3]  @ v[d]
	eor R6, R4        @ v[d] ^ v[a]
	ror R6, R6, #16   @ ROTR32 16
	str R6, [R7, R3]  @ save v[b]
	@ free R3, R4, R6

	@ v[c] = v[c] + v[d]
	ldrb R3, [R0, #2] @ c = gvec[2]
	ldr R4, [R7, R3]  @ v[c]
	add R4, R6        @ v[c] + v[d]
	str R4, [R7, R3]  @ save v[c]
	@ free R6

	@ v[b] = ROTR32(v[b] ^ v[c], 12)
	ldrb R3, [R0, #1] @ b = gvec[1]
	eor R5, R4        @ v[b] ^ v[c]
	ror R5, #12
	str R5, [R7, R3]
	@ free R3, R4

	@ v[a] = v[a] + v[b] + y
	ldrb R3, [R0]     @ a = gvec[0]
	ldr R4, [R7, R3]  @ v[a]
	add R4, R5        @ v[a] += v[b]
	ldrb R6, [R2], #1 @ *(sigma++)
	ldr R6, [R1, R6]  @ y
	add R4, R6        @ v[a] += y
	str R4, [R7, R3]  @ save v[a]
	@ free R3, R6

	@ v[d] = ROTR32(v[d] ^ v[a], 8)
	ldrb R3, [R0, #3] @ d = gvec[3]
	ldr R6, [R7, R3]  @ v[d]
	eor R6, R4        @ v[d] + v[a]
	ror R6, R6, #8    @ ROTR32 8
	str R6, [R7, R3]  @ save v[d]
	@ free R3, R4, R6

	@ v[c] = v[c] + v[d]
	ldrb R3, [R0, #2] @ c = gvec[2]
	ldr R4, [R7, R3]  @ v[c]
	add R4, R6        @ v[c] + v[d]
	str R4, [R7, R3]  @ save v[c]
	@ free R6

	@ v[b] = ROTR32(v[b] ^ v[c], 7)
	ldrb R3, [R0, #1] @ b = gvec[1]
	eor R5, R4        @ v[b] ^ v[c]
	ror R5, R5, #7    @ ROTR32 7
	str R5, [R7, R3]  @ save v[b]
	@ free R3, R4, R5

	@ TODO: increase pointers [ ]
	add R0, #4 @ gvec += 4

	@ function epilogue
	bx lr @ jump back

	.section .rodata
gvec:
	.byte 0
	.byte 16
	.byte 32
	.byte 48
	.byte 4
	.byte 20
	.byte 36
	.byte 52
	.byte 8
	.byte 24
	.byte 40
	.byte 56
	.byte 12
	.byte 28
	.byte 44
	.byte 60
	.byte 0
	.byte 20
	.byte 40
	.byte 60
	.byte 4
	.byte 24
	.byte 44
	.byte 48
	.byte 8
	.byte 28
	.byte 32
	.byte 52
	.byte 12
	.byte 16
	.byte 36
	.byte 56

iv: @ 8-element initialization vector
	.word 0x6A09E667
	.word 0xBB67AE85
	.word 0x3C6EF372
	.word 0xA54FF53A
	.word 0x510E527F
	.word 0x9B05688C
	.word 0x1F83D9AB
	.word 0x5BE0CD19

sigma: @ 10x16 sigma matrix
@ row 1
	.byte 0
	.byte 4
	.byte 8
	.byte 12
	.byte 16
	.byte 20
	.byte 24
	.byte 28
	.byte 32
	.byte 36
	.byte 40
	.byte 44
	.byte 48
	.byte 52
	.byte 56
	.byte 60
@ row 2
	.byte 56
	.byte 40
	.byte 16
	.byte 32
	.byte 36
	.byte 60
	.byte 52
	.byte 24
	.byte 4
	.byte 48
	.byte 0
	.byte 8
	.byte 44
	.byte 28
	.byte 20
	.byte 12
@ row 3
	.byte 44
	.byte 32
	.byte 48
	.byte 0
	.byte 20
	.byte 8
	.byte 60
	.byte 52
	.byte 40
	.byte 56
	.byte 12
	.byte 24
	.byte 28
	.byte 4
	.byte 36
	.byte 16
@ row 4
	.byte 28
	.byte 36
	.byte 12
	.byte 4
	.byte 52
	.byte 48
	.byte 44
	.byte 56
	.byte 8
	.byte 24
	.byte 20
	.byte 40
	.byte 16
	.byte 0
	.byte 60
	.byte 32
@ row 5
	.byte 36
	.byte 0
	.byte 20
	.byte 28
	.byte 8
	.byte 16
	.byte 40
	.byte 60
	.byte 56
	.byte 4
	.byte 44
	.byte 48
	.byte 24
	.byte 32
	.byte 12
	.byte 52
@ row 6
	.byte 8
	.byte 48
	.byte 24
	.byte 40
	.byte 0
	.byte 44
	.byte 32
	.byte 12
	.byte 16
	.byte 52
	.byte 28
	.byte 20
	.byte 60
	.byte 56
	.byte 4
	.byte 36
@ row 7
	.byte 48
	.byte 20
	.byte 4
	.byte 60
	.byte 56
	.byte 52
	.byte 16
	.byte 40
	.byte 0
	.byte 28
	.byte 24
	.byte 12
	.byte 36
	.byte 8
	.byte 32
	.byte 44
@ row 8
	.byte 52
	.byte 44
	.byte 28
	.byte 56
	.byte 48
	.byte 4
	.byte 12
	.byte 36
	.byte 20
	.byte 0
	.byte 60
	.byte 16
	.byte 32
	.byte 24
	.byte 8
	.byte 40
@ row 9
	.byte 24
	.byte 60
	.byte 56
	.byte 36
	.byte 44
	.byte 12
	.byte 0
	.byte 32
	.byte 48
	.byte 8
	.byte 52
	.byte 28
	.byte 4
	.byte 16
	.byte 40
	.byte 20
@ row 10
	.byte 40
	.byte 8
	.byte 32
	.byte 16
	.byte 28
	.byte 24
	.byte 4
	.byte 20
	.byte 60
	.byte 44
	.byte 36
	.byte 56
	.byte 12
	.byte 48
	.byte 52
	.byte 0
