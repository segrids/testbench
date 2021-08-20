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
from .apdu import Apdu
from .hexen import *


def loader(port):
    from .uart import Serial
    serial = Serial(port)
    return Loader(serial)

"""
class Loader()

Command interface to a target device (e.g. sam3x8e, ht32f52).

The target device is assumed to run a command handler for APDU instruction classes

'A' (Adapter)  | adapter.c
'L' (Loader)   | loader.c
'T' (Tester)   | testhandler.c
'D' (Debugger) | debugger.c

"""
class Loader():

    def __init__(self, serial):
        """Init Target with a APDU connection via a serial connection.

        Args:
            serial_or_port: Serial instance that is connected to the
            target with a baudrate of 115200 and a timeout period of 2 seconds 
        """
        self.apdu = Apdu(serial)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.disconnect()

    def reset(self):
        """Perform a hard processor reset."""
        self.apdu.serial.target_reset()

    def disconnect(self):
        """Close the target connection."""
        self.apdu.disconnect()
    
    ### Loader class commands ###

    def write_byte(self, address, value):
        """Write a byte to memory."""
        return self.write_data(address, bu8(value))

    def write_word(self, address, value):
        """Write a word to memory."""
        return self.write_indirect(address, bu32(value))

    def write_data(self, address, data):  # this is the direct write
        """Write a block of data to memory."""
        start = 0
        stop = 0
        while start < len(data):
            stop = min(start + 256, len(data))
            stdout.write('\rwriting {:#8x}...'.format(address + start))
            _, status = self.apdu.sendreceive(
                b'D',
                bu32(address + start) + data[start:stop])
            assert status == 0x9000
            stdout.flush()
            start = start + 256
        print(' done.')

    def load(self, address=0x20001000, path='ram.bin'):
        """Load binary at path into RAM."""
        with open(path, 'rb') as fd:
            data = fd.read()
        self.write_data(address, data)

    def write_indirect(self, address, words):  # important for flashing
        """Write a maximum of 64 words to memory.

        Only executes 32-bit write intructions as opposed to `write_data`s
        8-bit ones. Therefore slightly faster but also important for flashing.
        """
        assert len(words) % 4 == 0
        assert len(words) <= 256
        _, status = self.apdu.sendreceive(b'I', bu32(address) + words)
        return status

    def read_byte(self, address):
        """Read a byte from memory."""
        return ub8(self.read_data(address, size=1))

    def read_word(self, address):
        """Read a word from memory."""
        return ub32(self.read_data(address, size=4)[:4])

    def read_data(self, address, size):  # e.g. rt.read_block(0x2000000,128)
        """Read a block of any size from memory."""
        res, status = self.apdu.sendreceive(b'R', bu32(address), size)
        assert status == 0x9000
        return res

    def set_VTOR(self, address):
        """Set the VTOR."""
        return self.write_word(0xE000ED08, address)

    def get_VTOR(self):
        """Read the VTOR."""
        return self.read_word(0xE000ED08)

    def go(self, address, wait=True):
        """Execute code in memory.

        Args:
            address: An integer address to which the CPU should jump.
            wait: An optional boolean specifying whether to wait for the
                  executed function to return or return immediately.
        """
        self.apdu.sendreceive(b'G', bu32(address), read_response=wait)


"""
class Tester():

Extend methods of Loader class by a few ad-hoc tests.
"""
class Tester(Loader):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def blink(self):
        """Toggle pin B27 repeatedly."""
        _, status = self.apdu.sendreceive(b'B', cla=b'T')
        return status

    def echo(self, data):
        """Send test data that ought to be echoed back."""
        res, status = self.apdu.sendreceive(b'E', data, len(data), cla=b'T')
        assert status == 0x9000

        return res

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


"""
class Challenger():

Extend methods of Tester class by authentication methods.
"""
class Challenger(Tester):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def read_id(self):
        res, status = self.apdu.sendreceive(cla=b'R', ins=b'I', res_len=24)
        assert status == 0x9000
        return res

    def response(self, challenge=b'affe'*4):
        #if self.mcu == 'sam3x8e': # this target needs to be forced into a defined state
        #    self.apdu.serial.reset()
        #    time.sleep(0.1)
        #    self.read_id()
        #    time.sleep(0.1)
        res, status = self.apdu.sendreceive(cla=b'R', ins=b'C', data=challenge, res_len=16)
        assert status == 0x9000
        return res






