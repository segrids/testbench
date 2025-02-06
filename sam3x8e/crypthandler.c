#include <string.h>

#include "apdu.h"
#include "slave.h"  
#include "crypthandler.h"


struct tc_aes_key_sched_struct expected = {
		{
			0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c,
			0xa0fafe17, 0x88542cb1, 0x23a33939, 0x2a6c7605,
			0xf2c295f2, 0x7a96b943, 0x5935807a, 0x7359f67f,
			0x3d80477d, 0x4716fe3e, 0x1e237e44, 0x6d7a883b,
			0xef44a541, 0xa8525b7f, 0xb671253b, 0xdb0bad00,
			0xd4d1c6f8, 0x7c839d87, 0xcaf2b8bc, 0x11f915bc,
			0x6d88a37a, 0x110b3efd, 0xdbf98641, 0xca0093fd,
			0x4e54f70e, 0x5f5fc9f3, 0x84a64fb2, 0x4ea6dc4f,
			0xead27321, 0xb58dbad2, 0x312bf560, 0x7f8d292f,
			0xac7766f3, 0x19fadc21, 0x28d12941, 0x575c006e,
			0xd014f9a8, 0xc9ee2589, 0xe13f0cc8, 0xb6630ca6
		}
	};

struct tc_aes_key_sched_struct sched;
    
uint8_t nist_key[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
uint8_t nist_input[16] = {
        0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
        0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
    };
uint8_t expected_output[16] = {
        0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb,
        0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32
    };

	
int test_1(void){
	uint8_t ciphertext[16];

	if (tc_aes128_set_encrypt_key(&sched, nist_key) == 0) {
		return 1;
	}
	if (sizeof(sched.words) != sizeof(expected.words)) {
		return 2;
	}
	for (int i=0; i<44; i++){
		if (sched.words[i] != expected.words[i]) {
			return 3;
		}
	}
	if (tc_aes_encrypt(ciphertext, nist_input, &sched) == 0) {
		return 4;
	}
	/*if (memcmp(expected.words, sched.words, sizeof(sched.words))) {
		return 3;
	}
	for (int i=0; i<16; i++){
		if (ciphertext[i] != expected_output[i]) {
			return 5;
		}
	}*/
	return 0;
}

int handle_crypt(void){
	uint16_t status = 0x9000;
	if (apdu.ins == 'T') { // AES key scedule test
		if (apdu.lc != 0){
			status = 0xcc78;
		} else if (apdu.le != 0){
			status = 0xcc79;
		} else {
			status += (uint16_t)test_1();
		}
	} else {
		status = 0xcc66; // unknown ins
	}	
	return slave_send_uint16(status);
}



