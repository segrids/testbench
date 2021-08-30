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
 
#include "loader.h"
#include "apdu.h"
#include "slave.h"
#include "eefc.h"
#include "blake2s.h"


/* handle_loader()
 * 
 * Command handler for APDU instruction class 'L'.
 * 
 * Instructions supported by loader_handler():
 * 
 * 'D' Data write: Write bytestring of len <= 256 to address.
 * 
 * 
 *   INS   |   LC                 |     DATA                    | LE
 *   ----- | -------------------- | --------------------------- | ---
 *   'D'   |   4 +len(bytestring) |  bu32(address) + bytestring |  0
 * 
 *
 * 'E' Echo: Echo bytestring.  Implemented as a check that UART communication is working properly.
 * 
 * INS   |   LC            |     DATA    | LE
 * ----- | --------------- | ----------- | ---------------
 * 'E'   | len(bytestring) | bytestring  | len(bytestring)
 * 
 * 
 * 'F' Erase Flash page and write page: Flash page of Flash plane whoose buffer has 
 *                                      formerly been writte with 'I' instruction.
 * 
 *   INS   |   LC |     DATA                  | LE
 *   ----- | ---- | ------------------------- | ---
 *   'F'   |   4  |  bu16(plane) + bu16(page) |  0
 * 
 * 
 * 'G' Go: Branch with link to address. If a function with endless loop is called, 
 *         no response is expected.
 * 
 *   INS   |   LC |     DATA       | LE
 *   ----- | ---- | -------------- | ---
 *   'G'   |   4  |  bu32(address) |  0
 * 
 * 'I' Integer write: Write bytestring word wise to address. Return error status code 0x6702, 
 *                    if length len(bytestring) \% 4 == 0
 * 
 *   INS   |   LC                 |     DATA                    | LE
 *   ----- | -------------------- | --------------------------- | ---
 *   'I'   |   4 +len(bytestring) |  bu32(address) + bytestring |  0
 * 
 * 
 * 'L' Blake2e
 * 
 *   INS   |   LC                 |     DATA                    | LE
 *   ----- | -------------------- | --------------------------- | ---
 *   'L'   |   TODO               | TODO                        |  TODO
 * 
 * 
 * 'P': Set GPNVM to select boot medium. Three select codes are supported: 0xff (ROM), 
 *      0 (Flash Plane 0), 1 (Flash Plane 1)
 * 
 * 
 *   INS   |   LC  |  DATA         | LE
 *   ----- | ----- | ------------- | ---
 *   'P'   |   1   |  bu8(select)  |  0
 * 
 * 'R' Read: Read le bytes of DATA from address
 * 
 *   INS   |   LC  |  DATA           | LE
 *   ----- | ----- | --------------- | ---
 *   'R'   |   4   |  bu32(address)  | le
 * 
 */
int handle_loader(void) {
	uint16_t i, value;
	uint16_t status = 0x9000;

	if (apdu.ins == 'D' ) { // data write
		uint32_t address = *(uint32_t *) apdu.data;
		uint8_t * p = (uint8_t *) address;
		uint8_t * data = (uint8_t *) apdu.data + 4;
		for (int i = 0; i < apdu.lc - 4; i++) {
			p[i] = data[i];
		}

	} else if (apdu.ins == 'E') {
		slave_send_data(apdu.data, apdu.le);

	} else if (apdu.ins == 'F' ) { // flash a page whoose buffer has formerly been writte with 'I' command
		Eefc* p_eefc;
		if (*(uint16_t *)apdu.data == 1){
			p_eefc = EEFC1;
		} else {
			p_eefc = EEFC0;
		}
		value = eefc_erase_and_write_page(p_eefc, *(uint16_t *)(apdu.data+2));
		slave_send_uint16(value);

	} else if (apdu.ins == 'G' ) { // go to a void function with void argument
		uint32_t address = *(uint32_t *) apdu.data;
		address |= 1;
		void (*fp)(void) = (void (*)(void)) address;
		fp();

	} else if (apdu.ins == 'I') { // integer write
		if (apdu.lc % 4 != 0){
			status = 0x6701;
		} else {
			uint32_t address = *(uint32_t *) apdu.data;
			uint32_t * p = (uint32_t *) address;

			uint32_t * data = (uint32_t *) apdu.data + 1;

			for (i = 0; i < apdu.lc / 4 - 1; i++) {
				p[i] = data[i];
			}
		}

	} else if (apdu.ins == 'L') { // blake2s
		uint32_t address = *(uint32_t *) apdu.data;
		uint32_t inlen = *(((uint32_t *) apdu.data) + 1);
		uint8_t hash[32];
		blake2s(hash, (uint32_t *) address, inlen);
		slave_send_data(hash, 32);

	} else if (apdu.ins == 'P') { // write/clear GPNVM.1
		if (*apdu.data == 0xff) {
			eefc_clear_gpnvm_bit(1);
		} else if (*apdu.data == 0x00) {
			eefc_set_gpnvm_bit(1);
		} else if (*apdu.data == 0x01) {
			eefc_set_gpnvm_bit(1);
			eefc_set_gpnvm_bit(2);
		} else {
			status = 0x6A80;
		}
		uint8_t newValue;
		eefc_get_gpnvm(&newValue);
		slave_send_uint8(newValue); // Response data => Le=1

	} else if (apdu.ins == 'R') { // read
		uint32_t address = *(uint32_t *) apdu.data;
		slave_send_data((uint8_t *)address, apdu.le);

	} else {
	        perror((void *)handle_loader, "Unknown instruction!", apdu.ins);
		status = 0x6D00;
	}

	return slave_send_uint16(status);
}

