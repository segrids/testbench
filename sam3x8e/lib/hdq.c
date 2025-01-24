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
 
#include "hdq.h"
#include "pio.h"
#include "tc.h"

struct hdq_t hdq;

/* hdq_init()
 *
 * Initialize Arduino Due pins 14 and 15 for hdq.
 **/
int hdq_init(Hdq * p_hdq, int number) {
        if (number == 0){
                p_hdq->HDQ_TC = TC0;
                p_hdq->HDQ_PIO = PIOD;
                p_hdq->HDQ_PIN = 4;
                p_hdq->HDQ_POUT = 5;
		tc_pmc_enable(0);
		hdq_set_pin(p_hdq);
	} else {
		return -1; //no HDQ interface foreseen for the given number
	}
	// initialize TC0 with prescaler 8, then 3 clocks per us
	tc_init_capture_mode(p_hdq->HDQ_TC, 1000000, 1);
	return 0;
}

/* hdq_get_pin()
 *
 * Read state of HDQ line (Arduino Due Pin 14)
 **/
int hdq_get_pin(Hdq *p_hdq){
	return (int)((pio_read_input_pins(p_hdq->HDQ_PIO) >> p_hdq->HDQ_PIN) & 1);
}

/* hdq_set_pin()
 *
 * Set HDQ pin (Arduino Due pin 15) to open drain
 **/
void hdq_set_pin(Hdq *p_hdq){
	pio_set_output_pins(p_hdq->HDQ_PIO, 1 << p_hdq->HDQ_POUT);
}

/* hdq_clear_pin()
 *
 * Drive HDQ pin (Arduino Due pin 15) low.
 **/
void hdq_clear_pin(Hdq *p_hdq){
	pio_clear_output_pins(p_hdq->HDQ_PIO, 1 << p_hdq->HDQ_POUT);
}

/* hdq_sleep()
 *
 * Sleep a number of miliseconds.
 **/
void hdq_sleep(Hdq *p_hdq, int tus){
	tc_reset(p_hdq->HDQ_TC);
	while (tc_get_value(p_hdq->HDQ_TC) < 3 * tus){}
}

/* hdq_detect_low_pulse()
 *
 * Detect_low_pulse on HDQ input line (PIOD.4) 
 * return the low time in us.  
 **/
int hdq_detect_low_pulse(Hdq *p_hdq, int *lowtime){
	if (hdq_get_pin(p_hdq) == 0){
		return -1; // Error: signal already low at function entry
	} 
	while (hdq_get_pin(p_hdq) == 1) {}
	tc_reset(p_hdq->HDQ_TC);
	while (hdq_get_pin(p_hdq) == 0) {}
	*lowtime = (int)tc_get_value(p_hdq->HDQ_TC);
	return 0;
}

/* hdq_receive_uint8()
 *
 * Receive a byte.
 **/
int hdq_receive_uint8(Hdq *p_hdq, uint8_t * byte){
	int bit = 0;
	int lowtime;
	*byte = 0;
	while (bit < 8) {
		if (hdq_detect_low_pulse(p_hdq, &lowtime)){ // cannot detect low pulse because signal already low
			while(1);
		} else if (lowtime < 3*10) { // don't handle too short lowtime
			while(1);
		} else if (lowtime < 3*65) { // one bit detected
			*byte += (1 << bit);
			bit += 1;
		} else if (lowtime < 3*160) { // zero bit detected
			//*byte += (0 << bit);
			bit += 1;
		} else if (bit > 0) { // break detected after having received first bit
			while(1);
		}
	}
	return 0;
}

/* hdq_apply_low_pulse()
 *
 * Pull down the HDQ line for the required lowtime.
 **/
int hdq_apply_low_pulse(Hdq *p_hdq, int lowtime){
	if (hdq_get_pin(p_hdq) == 0){
		return -1; // Error: signal already low at function entry
	} 
	hdq_clear_pin(p_hdq);
	hdq_sleep(p_hdq, lowtime);
	hdq_set_pin(p_hdq);
	return 0;
}

/* hdq_send_break()
 *
 * Pull down the HDW signal line for 250ms adn wait another 50ms.
 **/
int hdq_send_break(Hdq *p_hdq){
	int ret;
	ret = hdq_apply_low_pulse(p_hdq, 250);
	hdq_sleep(p_hdq, 50);
	return ret;
}

/* hdq_send_uint8()
 *
 * Send a byte.
 **/
int hdq_send_uint8(Hdq *p_hdq, uint8_t byte){
	for (int bit=0; bit < 8; bit++){
		if ((byte & (1 << bit)) == 0){
			if (hdq_apply_low_pulse(p_hdq, 100)){
				return bit; // indicate bit in case of error
			}
			hdq_sleep(p_hdq, 100);
		} else {
			if (hdq_apply_low_pulse(p_hdq, 40)){
				return bit; // indicate bit in case of error
			}
			hdq_sleep(p_hdq, 160);
		}
	}
	return 0;
}

/* hdq_master_send_data()
 *
 * Master send function.
 *
 * Note:The address argument has a different meaning in HDQ than in I2C. It doesn't 
 * identify a slave but a memory address ("command" in TI speach)
 *
 * Writing *buffer to "address" requires:
 * 	hdq_send_uint8(HDQ0, address | 0x80)
 * 	hdq_send_uint8(HDQ0, buffer[0]
 *      hdq_send_uint8(HDQ0, address+1 | 0x80)
 * 	hdq_send_uint8(HDQ0, buffer[1]
 *      ...
 **/
int hdq_master_send_data(Hdq *p_hdq, uint8_t address, uint8_t *buffer, uint32_t buffer_len){
	// HDQ protocol requires most significant bit one for write operations
	address |= 0x80; 	
	for (int i=0; i<buffer_len; i++){
		if (hdq_send_break(p_hdq)){
			return -1;
		}
		if (hdq_send_uint8(p_hdq, address + i)){
			return -2;
		}
		if (hdq_send_uint8(p_hdq, buffer[i])){
			return -3;
		}
	}
	return 0;
}

/* hdq_master_receive_data()
 *
 * Master receive function.
 **/
int hdq_master_receive_data(Hdq *p_hdq, uint8_t address, uint8_t *buffer, uint32_t buffer_len){
	for (int i=0; i<buffer_len; i++){
		if (hdq_send_break(p_hdq)){
			return -1;
		}
		if (hdq_send_uint8(p_hdq, address + i)){
			return -2;
		}
		if (hdq_receive_uint8(p_hdq, buffer + i)){
			return -3;
		}
	}
	return 0;
}

/* hdq_close()
 *
 * No need to close the interface so far.
 **/
void hdq_close(Hdq *p_hdq){
	//todo
	return;
}

