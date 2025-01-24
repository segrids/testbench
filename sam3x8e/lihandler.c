
#include "li.h"    // 1024-bit Integer Lib
#include "apdu.h"
#include "slave.h"  

int handle_li(void){
	uint16_t status = 0x9000;
	if (apdu.ins == 'D') { // Write li.d
		if (apdu.lc != 128){
			status = 0x8878;
		} else {
			for (int i = 0; i < 32; i++) {
				li.d[i] = ((uint32_t *)apdu.data)[i];
			}
		}
	} else if (apdu.ins == 'd') { // Read li.d
		if (apdu.le != 128){
			status = 0x8879;
		} else {
			for (int i = 0; i < 32; i++) {
				slave_send_uint32(li.d[i]);
			}
		}
	} else if (apdu.ins == 'N') { // Write li.n
		if (apdu.lc != 128){
			status = 0x8878;
		} else {
			for (int i = 0; i < 32; i++) {
				li.n[i] = ((uint32_t *)apdu.data)[i];
			}
		}
	} else if (apdu.ins == 'n') { // Read li.n
		if (apdu.le != 128){
			status = 0x8879;
		} else {
			for (int i = 0; i < 32; i++) {
				slave_send_uint32(li.n[i]);
			}
		}
	} else if (apdu.ins == 'X') { // Write li.x
		if (apdu.lc != 128){
			status = 0x8878;
		} else {
			for (int i = 0; i < 32; i++) {
				li.x[i] = ((uint32_t *)apdu.data)[i];
			}
		}
	} else if (apdu.ins == 'x') { // Read li.x
		if (apdu.le != 128){
			status = 0x8879;
		} else {
			for (int i = 0; i < 32; i++) {
				slave_send_uint32(li.x[i]);
			}
		}
	} else if (apdu.ins == 'y') { // Read li.y
		if (apdu.le != 128){
			status = 0x8879;
		} else {
			for (int i = 0; i < 32; i++) {
				slave_send_uint32(li.y[i]);
			}
		}
	} else if (apdu.ins == 'Y') { // Write li.y
		if (apdu.lc != 128){
			status = 0x8878;
		} else {
			for (int i = 0; i < 32; i++) {
				li.y[i] = ((uint32_t *)apdu.data)[i];
			}
		}
	} else if (apdu.ins == 'b') { // Read li.buf
		if (apdu.le != 256){
			status = 0x8879;
		} else {
			for (int i = 0; i < 64; i++) {
				slave_send_uint32(li.buf[i]);
			}
		}
	} else if (apdu.ins == 'B') { // Write li.buf low 1024bits
		if (apdu.lc != 128){
			status = 0x8878;
		} else {
			for (int i = 0; i < 32; i++) {
				li.buf[i] = ((uint32_t *)apdu.data)[i];
			}
		}
	} else if (apdu.ins == 'H') { // Write li.buf high 1024 bits
		if (apdu.lc != 128){
			status = 0x8878;
		} else {
			for (int i = 0; i < 32; i++) {
				li.buf[i+32] = ((uint32_t *)apdu.data)[i];
			}
		}
	} else if (apdu.ins == 'g') { // get bit of li.d
		if (apdu.le != 4){
			status = 0x8479;
		} else if (apdu.lc != 4){
			status = 0x8579;
		} else {
			int bit = ((uint32_t *)apdu.data)[0];
			int value = li_getbit(li.d, bit);
			slave_send_uint32((uint32_t)value);
		}
	} else if (apdu.ins == 'l') { // Get (bitlen(li.y), bitlen(li.x), bitlen(li.d), bitlen(li.n))
		if (apdu.le != 20){
			status = 0x8879;
		} else {
			slave_send_uint32((uint32_t)li_bitlen(li.y, 32));
			slave_send_uint32((uint32_t)li_bitlen(li.x, 32));
			slave_send_uint32((uint32_t)li_bitlen(li.d, 32));
			slave_send_uint32((uint32_t)li_bitlen(li.n, 32));
			slave_send_uint32((uint32_t)li_bitlen(li.buf, 64));
		}
	} else if (apdu.ins == 'c') { // copy x to y, n to buf for testing
		if (apdu.le != 0){
			status = 0x8c79;
		} else {
			li_copy(li.y, li.x, 32);
			li_copy(li.buf, li.n, 32);
		}
	} else if (apdu.ins == 'e') { // erase x,y,buf for testing 
		if (apdu.le != 0){
			status = 0x8e79;
		} else {
			li_clear(li.x, 32);
			li_clear(li.y, 32);
			li_clear(li.buf, 64);
		}
	} else if (apdu.ins == 's') { // get shifted word of n
		if (apdu.le != 4){
			status = 0x8579;
		} else {
			int i = ((int *)apdu.data)[0];
			int shift = ((int *)apdu.data)[1];
			uint32_t value = li_get_word_shifted(li.n, 32, i, shift);
			slave_send_uint32(value);
		}
	} else if (apdu.ins == 'a') { // buf = y + (x << shift)
		if (apdu.lc != 4){
			status = 0x8a78;
		} else if (apdu.le != 0){
			status = 0x8a79;
		} else {
			int shift = ((int *)apdu.data)[0];
			li_clear(li.buf, 64);
			li_copy(li.buf, li.y, 32);
			li_add_shifted(li.buf, 64, li.x, 32, shift);
		}
	} else if (apdu.ins == 'k') { // buf >= (n << shift
		if (apdu.lc != 4){
			status = 0x8478;
		} else if (apdu.le != 4){
			status = 0x8479;
		} else {
			int shift = ((int *)apdu.data)[0];
			int ret = li_geq_shifted(li.buf, 64, li.n, 32, shift);
			slave_send_uint32((uint32_t)ret);
		}
	} else if (apdu.ins == 'm') { // buf = y - (x << shift)
		if (apdu.lc != 4){
			status = 0x8a78;
		} else if (apdu.le != 0){
			status = 0x8a79;
		} else {
			int shift = ((int *)apdu.data)[0];
			li_clear(li.buf, 64);
			li_copy(li.buf, li.y, 32);
			li_sub_shifted(li.buf, 64, li.x, 32, shift);
		}
	} else if (apdu.ins == 'q') { // buf = buf % n
		if (apdu.lc != 0){
			status = 0x8978;
		} else if (apdu.le != 0){
			status = 0x8979;
		} else {
			li_mod(li.buf, 64, li.n, 32);
		}
	} else if (apdu.ins == '*') { // buf = x*y
		if (apdu.lc != 0){
			status = 0x8078;
		} else if (apdu.le != 0){
			status = 0x8079;
		} else {
			li_mul(li.buf, 64, li.y, 32, li.x, 32);
		}
	} else if (apdu.ins == 'p') { // y = x**d % n
		if (apdu.lc != 4){
			status = 0x8778;
		} else if (apdu.le != 0){
			status = 0x8779;
		} else {
			int wordlen = ((int *)apdu.data)[0];
			li_modexp(wordlen);
		}
	} else {
		status = 0x6666; // unknown ins
	}	
	return slave_send_uint16(status);
}



