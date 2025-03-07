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
 
#include "pdma.h"

void pdma_transfer(pdma_Channel * channel, uint32_t * source, uint32_t * dest, uint16_t num_words) {
	int source_increment = (uint32_t) source < 0x40000000;
	int dest_increment = (uint32_t) dest < 0x40000000;

	channel->SADR = (uint32_t) source;
	channel->DADR = (uint32_t) dest;
	channel->TSR = (num_words << 16) | 0x01;

	if (source_increment && dest_increment) {
		channel->CR = (0x02 << 2) | 0x3;
	}
}
