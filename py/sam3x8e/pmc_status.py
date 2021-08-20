""" MIT License

Copyright (c) 2021 by Frank Schuhmacher <frank.schuhmacher@segrids.com>

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

Author: Frank Schuhmacher
"""


pid_dict = {
0: 'SUPC',
1: 'RSTC',
2: 'RTC',
3: 'RTT',
4: 'WDG',
5: 'PMC',
6: 'EEFC0',
7: 'EEFC1',
8: 'UART',
9: 'SMC_SDRAMC',
10: 'SDRAMC',
11: 'PIOA',
12: 'PIOB',
13: 'PIOC',
14: 'PIOD',
15: 'PIOE',
16: 'PIOF',
17: 'USART0',
18: 'USART1',
19: 'USART2',
20: 'USART3',
21: 'HSMCI',
22: 'TWI0',
23: 'TWI1',
24: 'SPI0',
25: 'SPI1',
26: 'SSC',
27: 'TC0',
28: 'TC1',
29: 'TC2',
30: 'TC3',
31: 'TC4',
32: 'TC5',
33: 'TC6',
34: 'TC7',
35: 'TC8',
36: 'PWM',
37: 'ADC',
38: 'DACC',
39: 'DMAC',
40: 'UOTGHS',
41: 'TRNG',
42: 'EMAC',
43: 'CAN0',
44: 'CAN1'}

def always_on(pid):
    return (pid < 9) or (pid == 10)


def always_txt(pid):
    if always_on(pid):
        return " (as always)"
    else:
        return ""

"""
pmc_status()

Read registers of the sam3x8e to analyse the status of the Power Management Control PMC.

arg:
    t = an instance of RamLoader, Loader, Tester, Sam3x8e, or Debugger
"""
def pmc_status(t):
    ret = ''
    PMC_PCSR0 = t.read_word(0x400E0618)
    PMC_PCSR1 = t.read_word(0x400E0708)
    ret += '---------------- Clocked Peripherals ------------------\n'
    for i in range(32):
        if ((PMC_PCSR0 >> i) & 1):
            pid = i
            ret += "PID %d: " %pid + pid_dict[pid] + always_txt(pid) + '\n'
    for i in range(12):
        if ((PMC_PCSR1 >> i) & 1):
            pid = i + 32
            ret += "PID %d: " %pid + pid_dict[pid] + always_txt(pid) + '\n'
    return ret

