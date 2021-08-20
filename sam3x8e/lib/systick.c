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
 


#include "systick.h"

/* systick_enable() */
int systick_enable(Systick* p_systick, uint32_t period, uint32_t enable_interrupt, uint32_t div8){
	if (period < 1){
		return -1;
	}
	p_systick->LOAD = (period-1) & 0xffffff;
	p_systick->CTRL = ((~div8 & 1) << 2) | ((enable_interrupt & 1) << 1) | 1;
	return 0;
}

/* systick_disable() */
void systick_disable(Systick* p_systick){
	p_systick->CTRL = p_systick->CTRL & ~1;
}

/* systick_get_value() */
uint32_t systick_get_value(Systick* p_systick){
	return p_systick->VAL;
}

/* systick_clear_value() */
void systick_clear_value(Systick* p_systick){
	p_systick->VAL = 0; //any other value would do
}
