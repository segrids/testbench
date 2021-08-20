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

import sys, os
from glob import glob
from time import sleep
import serial

def port2mcu(port):
    if port.find('USB') > 0:
        return 'ht32f52'
    elif port.find('ACM') > 0:
        return 'sam3x8e'
    else:
        return -1

"""
class Serial()

Extend the methods of serial.Serial by
 - a target_reset method for sam3x8e or ht32f52 eval module
 - an erase method for sam3x8e

Baudrate fixed to 115200.
"""
class Serial(serial.Serial):

    def __init__(self, port, mcu='', **kwargs):
        '''Initialize a serial object without opening it.'''
        super(Serial, self).__init__(**kwargs)
        self.port = port
        self.baudrate = 115200
        self.exclusive = False # only one instance can be exclusive
        self.timeout = 2
        if mcu == '':
            self.mcu = port2mcu(port)
        else:
            self.mcu = mcu
        self.open(port)

    def open(self, port=''):
        if self.mcu == 'ht32f52':
            self.setDTR(0) # pull up nRST
        if port == '':
            port = self.port
        super(Serial, self).open()


    def target_reset(self):
        if not self.is_open:
            self.open()
        if self.mcu == 'ht32f52':
            self.setDTR(1)
            sleep(0.1)
            self.setDTR(0)
        elif self.mcu == 'sam3x8e':
            self.close()
            sleep(0.4)
            self.open()
        else:
            return -1
        
    def erase(self):
        if self.mcu == 'sam3x8e':
            baudrate = self.baudrate
            self.close()
            self.baudrate = 1200
            self.open()
            sleep(0.1)
            self.close()
            self.baudrate = baudrate
            self.open
        else:
            return -1

