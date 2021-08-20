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

from ..loader import Tester
from ..hexen import *
from time import sleep
from progress.bar import Bar


def sam3x8e(port):
    from ..uart import Serial
    s = Serial(port)
    return Sam3x8e(s)

"""
Extend Loader / Tester methods by sam3x8e specific methods
"""
class Sam3x8e(Tester):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def blake2s(self, address, length):
        """Hash a section of memory using the blake2s algorithm.

        Args:
            address: The integer address of the block to hash.
            length: The integer length of the block. Has to be a multiple of
                64.

        Returns:
            32 bytes containing the 256-bit hash digest.
        """
        # TODO: assert length is multiple of 64?
        res, status = self.apdu.sendreceive(b'L', bu32(address) + bu32(length), 32)
        assert status == 0x9000
        return res

    @staticmethod
    def _page_address(plane, page):
        assert page < 1024
        if plane == 0:
            return 0x80000 + page * 256
        if plane == 1:
            return 0xC0000 + page * 256
        raise ValueError('There exists no plane {}.'.format(plane))

    def flash_data(self, data, plane, start_page):
        if len(data) % 256:
            data = data + b'\x00' * (256 - len(data) % 256)
        bar = Bar('Flashing', max=len(data)//256)
        for i in range(len(data) // 256):
            bar.next()
            page = start_page + i
            address = self._page_address(plane, page)
            assert self.write_indirect(address,
                                       data[256 * i:256 * (i + 1)]) == 0x9000
            sleep(0.1)
            # lance erasePageAndWritePage command
            self.apdu.sendreceive(b'F', bu16(plane) + bu16(page), res_len=2)

    def flash(self,
              path='sam3x8e/flash.bin',
              go=True,
              plane=0,
              start_page=0,
              set_GPNVM=True):
        """Flash a binary image onto the target.

        Args:
            path: An optional string path to the binary image.
            go: An optional boolean specifying whether to run the program's
                reset interrupt vector.
            plane: An optional integer index of the selected flash plane.
            start_page: An optional integer index of the first page to flash on
                the selected plane.
            set_GPNVM: An optional boolean specifying whether to set the GPNVM
                to boot from the selected plane after a CPU reset.
        """
        with open(path, 'rb') as fd:
            data = fd.read()
        print("going to flash file %s (%d bytes)" %(path, len(data)))  
        self.flash_data(data, plane, start_page)
        sleep(1)
        if set_GPNVM:
            self.set_GPNVM(plane)
        if go:
            self.go(ub32(data[4:8]), wait=False)

    def set_GPNVM(self, plane):
        """Set the GPNVM to boot from given plane on reset."""
        res, status = self.apdu.sendreceive(b'P', bu8(plane), 1)
        assert status == 0x9000
        return res

    def clear_GPNVM(self):
        """Set the GPNVM to run the ROM code on reset."""
        return self.set_GPNVM(plane=0xff)

    def unique_id(self): #sam3x8e specific => read_id
        """Read the sam3x8e's unique ID."""
        self.write_word(0x400E0C04, 0x5A00000e)
        assert self.read_word(0x400E0C08) == 0x0
        ret = self.read_data(0xC0000, 16)
        self.write_word(0x400E0C04, 0x5A00000f)
        assert self.read_word(0x400E0C08) == 0x01
        return ret

