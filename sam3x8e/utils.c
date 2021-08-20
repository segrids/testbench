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
 
#include "utils.h"
#include "rtt.h"
#include "pio.h"

/* provide a few utils */

void copy_words(uint32_t target, uint32_t source, uint32_t len){
	uint32_t * p_source = (uint32_t *) source;
	uint32_t * p_target = (uint32_t *) target;
	for (int i = 0; i < len; i++) {
		p_target[i] = p_source[i];
	}
}

void copy_bytes(uint32_t target, uint32_t source, uint32_t len){
	uint8_t * p_source = (uint8_t *) source;
	uint8_t * p_target = (uint8_t *) target;
	for (int i = 0; i < len; i++) {
		p_target[i] = p_source[i];
	}
}

void utils_set_led(void){
	pio_set_output_pins(PIOB, 1<<27);
}

void utils_clear_led(void){
	pio_clear_output_pins(PIOB, 1<<27);
}

void utils_toggle_led(void){
	pio_toggle_output_pins(PIOB, 1<<27);
}

void utils_set_red(void){
	pio_set_output_pins(PIOB, 1<<14);
}

void utils_clear_red(void){
	pio_clear_output_pins(PIOB, 1<<14);
}

void utils_toggle_red(void){
	pio_toggle_output_pins(PIOB, 1<<14);
}

//__attribute__ ((section(".ramcode")))
void utils_blink(int times, int delay){
	Pio* p_pio = PIOB;
	Rtt* p_rtt = RTT;
	for (int i=0; i<times; i++){
		pio_set_output_pins(p_pio, 1<<27);
		rtt_sleep(p_rtt, delay);
		pio_clear_output_pins(p_pio, 1<<27);
		rtt_sleep(p_rtt, delay);
	}
}


void utils_error(int code){
	utils_set_red();
	perror((void *)utils_error, "Error!", code);
	while(1){}
}
