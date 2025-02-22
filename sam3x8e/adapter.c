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

Author: SEGRIDS GmbH <www.segrids.com>
*/

#include "adapter.h"
#include "pio.h"
#include "rtt.h"
#include "systick.h"
#include "slave.h"
#include "master.h"
#include "apdu.h"

/* handle_adapter()
 *
 * Arduino Due as adapter between slave_interface and master_interface
 *
 * handles instructions
 * 	'O' (Open):    Initialize a master interface specified by `protocol = arg[0]` 
 * 	               with configuration selected by `config = arg[1]`
 *      'C' (Close):   Close the slave interface
 *	'R' (Read):    Receive `apdu.le` bytes on the slave interface and send 
 *	               it to the master interface
 *	'W' (Write):   Send `apdu.data[1:]` to the slave with `slave_address = apdu.data[0]`
 *	'w' (write):   Same as 'W' but additionally set GPIO PIN to trigger scope.
 */
int handle_adapter(void) {
	uint16_t status = 0;
	// since the adapter also shall serve as debugger, use the systick
	// as a watchdog timer. The systick interrupt will restart the apdu loop.
	//systick_enable(SYSTICK, 1024*1024, 1, 1); //systick with interrupt clocked with MCK/8
	if (apdu.ins == 'O') {
		uint8_t protocol = apdu.data[0]; // in {'U', 'V', 'I', 'S', 'H'}
		uint8_t config = apdu.data[1];
		status = (uint16_t)master_init(protocol, config);

	} else if (apdu.ins == 'C') {
		master_close();

	} else if (apdu.ins == 'R') {
		uint8_t buffer[apdu.le];
		uint8_t slave_address = apdu.data[0];
		status = (uint16_t)master_receive_data(slave_address, buffer, apdu.le);
		slave_send_data(buffer, apdu.le);

	} else if (apdu.ins == 'T') {
		pio_clear_output_pins(PIOC, 1 << 25);
		rtt_sleep(RTT, 50); // sleep 50ms
		pio_set_output_pins(PIOC, 1 << 25);

	} else if (apdu.ins == 'W') {
		uint8_t slave_address = apdu.data[0];
		status = master_send_data(slave_address, apdu.data+1, apdu.lc-1);

	} else if (apdu.ins == 'w') { 
		uint8_t slave_address = apdu.data[0];
		// set GPIO pin
		pio_set_output_pins(PIOC, 1 << 21);
		status = master_send_data(slave_address, apdu.data+1, apdu.lc-1);
		// clear GPIO pin
		pio_clear_output_pins(PIOC, 1 << 21);
	} else {
		perror((void *)handle_adapter, "Unknown instruction!", apdu.ins);
		status = 0x6D00;
	}
	//systick_disable(SYSTICK); // the target at the adapter did not cause an infinite wait loop
	if (status == 0){
		status = 0x9000;
	}
	slave_send_uint16(status);
	return 0;
}

