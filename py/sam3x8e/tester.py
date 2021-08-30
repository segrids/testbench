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

from sys import stdout
import time

from . import uart
from ..apdu import Apdu
from .programmer import Programmer
from .hexen import *


def Tester(port):
    from .uart import Serial
    serial = Serial(port)
    return Tester(serial)

"""
class Tester()

Command interface to a sam3x8e target device.

The target device is assumed to run a command handler for APDU instruction classes

'L' (Loader)   | loader.c
'T' (Tester)   | tester.c

Extend methods of Programmer class by tests.
"""
class Tester(Programmer):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def blink(self):
        """Toggle pin B27 repeatedly."""
        _, status = self.apdu.sendreceive(b'B', cla=b'T')
        return status

    def set_led(self, value):
        _, status = self.apdu.sendreceive(b'l', bu8(value), cla=b'T')
        assert status == 0x9000

    def pc(self):
        """Read the address of the uart_send_uint8 function.

        Useful to detect if program is executing in flash or RAM.
        """
        res, status = self.apdu.sendreceive(b'W', b'', 4, cla=b'T')
        assert status == 0x9000
        return ub32(res)

    ### Test SPI slave commands (Attention: MOSI to MOSI, MISO to MISO)
    def spi_open(self):
        res, status = self.apdu.sendreceive(cla=b'T', ins=b'o', res_len=0)
        assert status == 0x9000

    def spi_send(self, data):
        res, status = self.apdu.sendreceive(cla=b'T', ins=b'w', data=data, res_len=0)
        assert status == 0x9000

    def spi_receive(self, length):
        res, status = self.apdu.sendreceive(cla=b'T', ins=b'r', res_len=length)
        assert status == 0x9000
        return res

    ### Test I2C slave commands
    def i2c_open(self):
        res, status = self.apdu.sendreceive(cla=b'T', ins=b'O', res_len=0)
        assert status == 0x9000

    def i2c_send(self, data):
        res, status = self.apdu.sendreceive(cla=b'T', ins=b'W', data=data, res_len=0)
        assert status == 0x9000

    def i2c_receive(self, length):
        res, status = self.apdu.sendreceive(cla=b'T', ins=b'R', res_len=length)
        assert status == 0x9000
        return res






