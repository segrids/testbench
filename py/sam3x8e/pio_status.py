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


due = dict()

due['A'] = {
0: 'CANTX',
1: 'CANRX',
2: 'A7',
3: 'A6',
4: 'A5',
6: 'A4',
22: 'A3',
23: 'A2',
24: 'A1',
16: 'A0',
10: '19 (USART0, RX1)',
11: '18 (USART0, TX1)',
12: '17 (USART1, RX2)',
13: '16 (USART1, TX2)',
8: '0 (UART, RX0)',
9: '1 (UART, TX0)',
29: '4 (shortend with C.26)',
28: '10 (shortend with C.29)',
17: 'SDA1',
18: 'SCL1',
19: '42',
15: '24',
14: '23',
19: '42',
20: '43'
}

due['B'] = {
15: 'DAC0',
16: 'DAC1',
17: 'A8',
18: 'A9',
19: 'A10',
20: 'A11',
13: '21 (TWI1, SCL)',
12: '20 (TWI1, SDA)',
25: '2',
27: '13',
21: '52',
26: '22'
}

due['C'] = {
28: '3',
26: '4 (shortend with A.29)',
25: '5',
24: '6',
23: '7',
22: '8',
21: '9',
29: '10 (shortend with A.28)',
6: '38',
8: '40',
19: '44',
17: '46',
15: '48',
13: '50',
12: '51',
14: '49',
16: '47',
18: '45',
5: '37',
3: '35',
1: '33',
2: '34',
4: '36'
}

due['D'] = {
5: '15',
4: '14',
7: '11',
8: '12',
0: '25',
2: '27',
6: '29',
1: '26',
3: '28',
9: '30',
10: '32'
}


"""
pio_status()

check the pin assignments of the sam3x8e MCu

arg:
    t = an instance of Samba, Loader, Tester, Programmer, or Debugger
"""

def pio_status(t):
    # check which pins are assigned to PIO
    PIO_PSR = dict()
    PIO_PSR['A'] = t.read_word(0x400E0E08) # PIOA 
    PIO_PSR['B'] = t.read_word(0x400E1008) # PIOB 
    PIO_PSR['C'] = t.read_word(0x400E1208) # PIOC
    PIO_PSR['D'] = t.read_word(0x400E1408) # PIOD
  
    # check which pins are configures as output 
    PIO_OSR = dict()
    PIO_OSR['A'] = t.read_word(0x400E0E18)
    PIO_OSR['B'] = t.read_word(0x400E1018)
    PIO_OSR['C'] = t.read_word(0x400E1218)
    PIO_OSR['D'] = t.read_word(0x400E1418)

    # check output pin status
    PIO_ODSR = dict()
    PIO_ODSR['A'] = t.read_word(0x400E0E38)
    PIO_ODSR['B'] = t.read_word(0x400E1038)
    PIO_ODSR['C'] = t.read_word(0x400E1238)
    PIO_ODSR['D'] = t.read_word(0x400E1438)

    # read input pin status
    PIO_PDSR = dict()
    PIO_PDSR['A'] = t.read_word(0x400E0E3C)
    PIO_PDSR['B'] = t.read_word(0x400E103C)
    PIO_PDSR['C'] = t.read_word(0x400E123C)
    PIO_PDSR['D'] = t.read_word(0x400E143C)

    input_pins = []
    output_pins = []

    for port in ['A','B','C','D']:
        pins =  list(due[port].keys())
        pins.sort()
        for i in pins:
            if ((PIO_PSR[port] >> i) & 1): # assigned to PIO?
                if ((PIO_OSR[port] >> i) & 1): # output pin?
                    if ((PIO_ODSR[port] >> i) & 1): # output high?
                        output_pins += [(port, i, due[port][i], "high")]
                    else:
                        output_pins += [(port, i, due[port][i], "low")]
                else:
                    if ((PIO_PDSR[port] >> i) & 1): # input high?
                        input_pins += [(port, i, due[port][i], "high")]
                    else:
                        input_pins += [(port, i, due[port][i], "low")]
    
    print('Output Pins:')
    for pp in output_pins: print(pp)
    print('Input Pins:')
    for pp in input_pins: print(pp)

    print('---------------------------')

    print('\nThe folling pins are assigned to peripherals supported by our driver lib:') 
    PIO_ABSR_A = t.read_word(0x400E0E70)
    if PIO_PSR['A'] & (1 << 2) == 0 and PIO_ABSR_A & (1 << 2) == 0: # peripheral and peripheral A
        print("A2: TIOA1")
    if PIO_PSR['A'] & (1 << 3) == 0 and PIO_ABSR_A & (1 << 3) == 0: # peripheral and peripheral A
        print("A3: TIOB1")
    if PIO_PSR['A'] & (1 << 4) == 0 and PIO_ABSR_A & (1 << 4) == 0: # peripheral and peripheral A
        print("A4: TCLK (Timer/Counter) input")
    if PIO_PSR['A'] & (1 << 8) == 0 and PIO_ABSR_A & (1 << 8) == 0: # peripheral and peripheral A
        print("A8: UART RX")
    if PIO_PSR['A'] & (1 << 9) == 0 and PIO_ABSR_A & (1 << 9) == 0: # peripheral and peripheral A
        print("A9: UART TX")
    if PIO_PSR['A'] & (1 << 10) == 0 and PIO_ABSR_A & (1 << 10) == 0: # peripheral and peripheral A
        print("A10: USART0 RX")
    if PIO_PSR['A'] & (1 << 11) == 0 and PIO_ABSR_A & (1 << 11) == 0: # peripheral and peripheral A
        print("A11: USART0 TX")
    if PIO_PSR['A'] & (1 << 12) == 0 and PIO_ABSR_A & (1 << 12) == 0: # peripheral and peripheral A
        print("A12: USART1 RX")
    if PIO_PSR['A'] & (1 << 13) == 0 and PIO_ABSR_A & (1 << 13) == 0: # peripheral and peripheral A
        print("A13: USART1 TX")
    if PIO_PSR['A'] & (1 << 17) == 0 and PIO_ABSR_A & (1 << 17) == 0: # peripheral and peripheral A
        print("A17: I2C0 SCL")
    if PIO_PSR['A'] & (1 << 18) == 0 and PIO_ABSR_A & (1 << 18) == 0: # peripheral and peripheral A
        print("A18: I2C0 SDA")
    if PIO_PSR['A'] & (1 << 25) == 0 and PIO_ABSR_A & (1 << 25) == 0: # peripheral and peripheral A
        print("A25: SPI0 MISO")
    if PIO_PSR['A'] & (1 << 26) == 0 and PIO_ABSR_A & (1 << 26) == 0: # peripheral and peripheral A
        print("A26: SPI0 MOSI")
    if PIO_PSR['A'] & (1 << 27) == 0 and PIO_ABSR_A & (1 << 27) == 0: # peripheral and peripheral A
        print("A27: SPI0 SCKL")
    if PIO_PSR['A'] & (1 << 28) == 0 and PIO_ABSR_A & (1 << 28) == 0: # peripheral and peripheral A
        print("A28: SPI0 nSS")
    PIO_ABSR_B = t.read_word(0x400E1070)
    if PIO_PSR['B'] & (1 << 12) == 0 and PIO_ABSR_B & (1 << 12) == 0: # peripheral and peripheral A
        print("B12: I2C1 SDA")
    if PIO_PSR['B'] & (1 << 13) == 0 and PIO_ABSR_B & (1 << 13) == 0: # peripheral and peripheral A
        print("B13: I2C1 SCL")
    if PIO_PSR['B'] & (1 << 28) == 0 and PIO_ABSR_B & (1 << 28) == 0: # peripheral and peripheral A
        print("B28: SWD SWSCL")
    if PIO_PSR['B'] & (1 << 31) == 0 and PIO_ABSR_B & (1 << 31) == 0: # peripheral and peripheral A
        print("B31: SWD SWDIO")

