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

Author: David Kuehnemann
"""

from ..hexen import *
from .swd import SWD


"""
class AHB()

Communicate with the AHB Access Port on top of a SWD connection to allow for memory access.

Note: AHB and the CPU should not access registers at the same time. To read these
registers halt the Processor either manually or via a (soft- or hardware)
breakpoint.
"""
class AHB():
    CSW = 0x00
    TA = 0x04
    DRW = 0x0c

    def __init__(self, *args, **kwargs):
        self.swd = SWD(*args, **kwargs)

    def read_byte(self, addr):
        """Read a byte from the target's memory."""
        self.swd.write_dp(self.swd.SELECT, 0x00)
        self.swd.write_ap(self.CSW, 0x22000000)
        self.swd.write_ap(self.TA, addr)
        self.swd.read_ap(self.DRW)
        return (self.swd.read_ap(0x00) >> 8 * (addr % 4)) & 0xff

    def read_halfword(self, addr):
        """Read a halfword from the target's memory."""
        self.swd.write_dp(self.swd.SELECT, 0x00)
        self.swd.write_ap(self.CSW, 0x22000001)
        self.swd.write_ap(self.TA, addr)
        self.swd.read_ap(self.DRW)
        return (self.swd.read_ap(0x00) >> 8 * (addr % 4)) & 0xffff

    def read_word(self, addr):
        """Read a word from the target's memory."""
        self.swd.write_dp(self.swd.SELECT, 0x00)
        self.swd.write_ap(self.CSW, 0x22000012)
        self.swd.write_ap(self.TA, addr)
        self.swd.read_ap(self.DRW)
        return self.swd.read_ap(0x00)

    def write_byte(self, addr, val):
        """Write a byte to the target's memory."""
        self.swd.write_dp(self.swd.SELECT, 0x00)
        self.swd.write_ap(self.CSW, 0x22000000)
        self.swd.write_ap(self.TA, addr)
        self.swd.write_ap(self.DRW, val << 8 * (addr % 4))

    def write_halfword(self, addr, val):
        """Write a halfword to the target's memory."""
        self.swd.write_dp(self.swd.SELECT, 0x00)
        self.swd.write_ap(self.CSW, 0x22000001)
        self.swd.write_ap(self.TA, addr)
        self.swd.write_ap(self.DRW, val << 8 * (addr % 4))

    def write_word(self, addr, val):
        """Write a word to the target's memory."""
        self.swd.write_dp(self.swd.SELECT, 0x00)
        self.swd.write_ap(self.CSW, 0x22000012)
        self.swd.write_ap(self.TA, addr)
        self.swd.write_ap(self.DRW, val)

    def read_data(self, addr, size):
        """Read a block of the target's memory."""
        if size > 0x1000:
            raise ValueError(
                'block size of {:#x} exceeds maximum of 4kB'.format(size))
        data = b''
        if size >= 4:
            self.swd.write_dp(self.swd.SELECT, 0x00)
            self.swd.write_ap(self.CSW, 0x2200001a)
            self.swd.write_ap(self.TA, addr)
            self.swd.read_ap(self.DRW)
            for _ in range(size // 4):
                data += bu32(self.swd.read_ap(self.DRW))
        for offset in range(4 * (size // 4), size):
            data += bu8(self.read_byte(addr + offset))
        return data

    def write_data(self, addr, data):
        """Write a block to the target's memory."""
        self.swd.write_dp(self.swd.SELECT, 0x00)
        self.swd.write_ap(self.CSW, 0x2200001a)

        assert len(data) % 4 == 0
        blocksize = 0x400
        for offset in range(0, len(data), blocksize):
            block = data[offset:min(offset + blocksize, len(data))]
            self.swd.write_ap(self.TA, addr + offset)

            for start in range(0, len(block), 4):
                self.swd.write_ap(self.DRW, ub32(block[start:start + 4]))


