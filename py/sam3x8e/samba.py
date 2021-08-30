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

Authors: 
    Frank Schuhmacher
    David Kuehnemann
"""

import struct
import time
import serial

from ..uart import Serial
from ..crc16 import crc16citt
from ..hexen import *


def erase(port='/dev/ttyACM0'):
    """Erase the flash of and reset the target on the specified port."""
    s = serial.Serial(port=port, baudrate=1200)
    time.sleep(0.2)
    s.close()

def samba(port):
    serial = Serial(port)
    return Samba(serial)


"""
class Samba()

Communication with the SAM3x8e boot code.

When erased, a microcontroller using the SAM3x8e will run a boot program that
enables a user to perform actions like read/write RAM and execute code. This
functionality is wrapped in the class `Samba`.
"""
class Samba():

    def __init__(self, serial):
        """Init Samba and connect to target.

        Raises an exception if no connection is established after 3 tries.

        Args:
            serial_or_port: Either a Serial instance that is connected to the
                target or a string in which case a serial connection with a
                baudrate of 115200 and a timeout period of 3 seconds on that
                port is opened.

        Raises:
            IOError: No reponse from target on `port`.
        """
        self.serial = serial
        self._connect()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.disconnect()

    def reset(self):
        """Reset connection and boot program."""
        self._connect(target_reset=True)

    def _connect(self, target_reset=True):
        """Connect to target.

        Raises an exception if no connection is established after 3 tries.

        Raises:
            IOError: No response from target on `port`.
        """
        if target_reset:
            self.serial.target_reset()

        for _ in range(3):
            time.sleep(.6)
            self.serial.write(b'N#')  # set normal mode
            if self.serial.read_until(b'\n\r') == b'\n\r':
                break  # connection established
            else:
                self.serial.close()
                self.serial.open()
        else:
            raise IOError('No response from target on {}.'.format(
                self.serial.port))

        ###self.serial.timeout = 3

    def disconnect(self):
        """Close the connection to the target."""
        self.serial.close()

    def version(self):
        """Read and return the boot program's version string."""
        self.serial.write(b'V#')
        return self.serial.read(27)

    def read_data(self, address, size):
        """Read a block of bytes of any size from memory."""
        num_blocks = size // 128
        data = b''

        if num_blocks > 0:
            self.serial.write(('R%x,%x#' % (address, 128 * num_blocks)).encode())
            self.serial.write(b'C')

            for framecount in range(1, num_blocks + 1):
                frame = self.serial.read(1 + 1 + 1 + 128 + 2)
                (soh, blk, blkComp, chunk, crc) = struct.unpack(
                    '<BBB128s2s', frame)

                assert soh == 0x01
                assert blk == framecount
                assert blkComp == 0xff - blk
                assert crc16citt(chunk) == crc

                data += chunk
                self.serial.write(b'\x06')

            eot = self.serial.read(1) 
            if not eot == b'\x04':  # EOT - End of Transmission
                print('eot', eot)
            self.serial.write(b'\x06')

        for offset in range(128 * num_blocks, size):
            data += bu8(self.read_byte(address + offset))

        return data

    def read_word(self, address):
        """Read a word from memory."""
        self.serial.write(('w%x,#' % address).encode())
        ret = self.serial.read(4)
        return ub32(ret)

    def read_byte(self, address):
        """Read a byte from memory."""
        self.serial.write(('o%x,#' % address).encode())
        ret = self.serial.read(1)
        return ret[0]

    def write_byte(self, address, value):
        """Write a byte to memory."""
        self.serial.write(('O%x,%x#' % (address, value)).encode())

    def write_word(self, address, value):
        """Write a word to memory."""
        self.serial.write(('W%x,%x#' % (address, value)).encode())

    def write_data(self, address, data):
        num_blocks = len(data) // 128
        if num_blocks > 0:
            self.serial.write(('S%x,#' % address).encode())
            assert self.serial.read(1) == b'C'

            for i in range(0, num_blocks):
                time.sleep(0.05)
                framecount = i + 1  # framecount starts at 1
                start = 128 * i
                blob = data[start:start + 128]
                self.serial.write(b'\x01' + bu8(framecount) +
                                  bu8(255 - framecount) + blob + crc16citt(blob))
                ack = self.serial.read(1)
                assert ack == b'\x06'

            self.serial.write(b'\x04')
            ack = self.serial.read(1)
            assert ack == b'\x06'

        num_rest = len(data) % 128
        rest_bytes = data[-num_rest:]
        for offset, byte in enumerate(rest_bytes):
            addr = address + 128 * num_blocks + offset
            self.write_byte(addr, byte)

    def go(self, address):
        """Execute a program with vector table at given address."""
        self.serial.write(('G%x#' % address).encode())

    def set_VTOR(self, address):
        """Set the vector table offset register.

        Args:
            address: An integer address to which to set the VTOR. Has to be
                32-word aligned.
        """
        assert address & 0x3f == 0
        self.write_word(0xE000ED08, address)
        assert self.read_word(0xE000ED08) == address

    def load(self, address, path, go=True):
        """Load binary from disk into RAM and optionally start execution.

        Args:
            address: An integer address at which the binary should be placed.
            path: A string path to the binary.
            go: Optional boolean specifying if the program's reset handler
                should be executed.
        """
        # last 7 bits are always zero since VT needs to be at least 32 word aligned
        assert address & 0x3f == 0
        assert (address >> 28) == 2
        with open(path, 'rb') as fd:
            data = fd.read()
        if len(data) % 128:
            data = data + b'\x00' * (128 - len(data) % 128)
        self.write_data(address, data)
        if go:
            self.go(address)

"""
test_samba()

Test the methods of the Samba class.

Required that an Arduino Due is connected to port.
"""
def test_samba(port='/dev/ttyACM0', ramimage='sam3x8e/images/bootloader.bin', go=False):
    erase(port)
    # test sanity
    print("Test bootloader") 
    sb = samba(port)
    version = sb.version()
    assert version[0:3] == b'v1.'
    assert version[-2:] == b'\n\r'
    print("SAM_BA boot version:" , version[:-2])
    # test read methods 
    assert sb.read_word(0) == 0x20001000 # top of stack
    assert sb.read_word(4) == 0x10004d   # points to reset_handler in ROM
    sb.read_word(0x10000) == sb.read_word(0x0) # address 0 points to ROM
    sb.read_word(0x12000) == sb.read_word(0x10000) # ROM address space limited to 8k
    for data_len in [1,4,10,100,1000]:
        assert sb.read_data(0x80000,data_len) == b'\xff'*data_len
    for byte in range(4): # Least significant byte first test
        assert (sb.read_word(0x20001000) >> (8*byte)) & 0xff == sb.read_byte(0x20001000 + byte)
    # test write methods
    sb.write_byte(0x20002003, 0xaf) 
    sb.write_byte(0x20002002, 0xfe) 
    sb.write_byte(0x20002001, 0xca) 
    sb.write_byte(0x20002000, 0xfe) 
    assert sb.read_word(0x20002000) == 0xaffecafe
    data = b'text test text test text'
    sb.write_data(0x20002000, data)
    assert sb.read_data(0x20002000, len(data)) == data
    sb.write_data(0x20002000, data*10)
    assert sb.read_data(0x20002000, len(data)*10) == data*10
    # test reset, _connect and go methods
    sb._connect(target_reset=False)
    assert sb.version() == version
    for pointer in [0,1000,0x10000,0x80000,0x20000000,0x20002000]:
        sb.set_VTOR(0) # method contains assertions
    sb.go(0x0)
    sb._connect(target_reset = False)
    assert sb.version() == version
    # test load method
    if ramimage != '':
        sb.load(0x20071000, ramimage, go=False)
        with open(ramimage,'rb') as fd:
            data = fd.read()
        assert sb.read_data(0x20071000, len(data)) == data
    if go:
        sb.go(0x20071000)
        return sb.serial # return serial handler
