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
 
#include "wdt.h"



/* wdt_enable
 *
 * Enable Watchdog Timer.
 */
void wdt_enable(Wdt* p_wdt, int status){
	/* configure WDR_MR (Mode Register)
		bit 11-0: WDV WatchDog Value (0xfff is 16s, 0x100 is 1s)
		bit 13: WatchDog Reset Enable
		bit 14: Only Processor Reset
		bit 28: Stop WatchDog Timer in Debug Mode
	*/
	p_wdt->WDT_MR = (p_wdt->WDT_MR & ~0x8000) | (0x8000 & (~status << 15));
}

/*  wdt_reset()
 *
 * Reset Watchdog Timer to have a new time interval before the processor reset.
 */
void wdt_reset(Wdt* p_wdt){
	p_wdt->WDT_CR = (0xa5 << 24) | 0x01;
}

