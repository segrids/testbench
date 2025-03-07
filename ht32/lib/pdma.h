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
 
#ifndef PDMA_H_
#define PDMA_H_

#include "types.h"

typedef struct {
	volatile uint32_t CR;
	volatile uint32_t SADR;
	volatile uint32_t DADR;
	volatile const uint32_t Reserved;
	volatile uint32_t TSR;
	volatile const uint32_t CTSR;
} pdma_Channel;

#define pdma_Channel0 ((pdma_Channel *) 0x40090000)
#define pdma_Channel1 ((pdma_Channel *) 0x40090018)
#define pdma_Channel2 ((pdma_Channel *) 0x40090030)
#define pdma_Channel3 ((pdma_Channel *) 0x40090048)
#define pdma_Channel4 ((pdma_Channel *) 0x40090060)
#define pdma_Channel5 ((pdma_Channel *) 0x40090078)

void pdma_transfer(pdma_Channel * channel, uint32_t * source, uint32_t * dest, uint16_t num_words);

#endif
