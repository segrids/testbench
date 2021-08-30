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

from sys import exit, stdout
import time

from progress.bar import Bar

from ..hexen import *
from .. import uart
from ..loader import Loader



def programmer(port):
    from ..uart import Serial
    s = Serial(port)
    return Programmer(s)

"""
Extend Loader methods by sam3x8e specific programming methods
"""
class Programmer(Loader):

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
            time.sleep(0.1)
            # lance erasePageAndWritePage command
            self.apdu.sendreceive(b'F', bu16(plane) + bu16(page), res_len=2)
        bar.finish()

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
        time.sleep(1)
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

"""
program()

Program flash image into flash of the sam3x8e target.

Use Samba methods to load and execute the bootloader image.
Use Progarmmer methods to flash and execute the flash image.
"""
def program(port,
          image='sam/flash.bin', 
          erase=True, 
          reset=True, 
          verify=True,
          bootloader_image='sam3x8e/bootloader.bin', 
          plane=0, 
          boot_rom=False):

    from . import samba
    from ..blake2s import Blake2s

    if erase:
        print('Erasing...')
        stdout.flush()
        samba.erase(port)
        print(' done.')

    print('Initializing connection...')
    stdout.flush()
    # connect to boot code
    sb = samba.samba(port)
    assert sb.read_word(0) == 0x20001000
    print(' done.')

    print('Uploading bootloader...')
    stdout.flush()
    # load and run bootloader
    sb.load(0x20071000, bootloader_image, go=False)
    assert sb.read_word(0) == 0x20001000
    sb.go(0x20071000)
    time.sleep(0.01)
    print(' done.')

    print('Connecting to bootloader...')
    stdout.flush()
    # connect to bootloader
    assert isinstance(sb.serial, uart.Serial)
    p = Programmer(sb.serial)
    time.sleep(0.01)
    print(' done.')

    assert p.read_byte(0x80000) == 0xff

    print('Flashing...')
    stdout.flush()
    p.flash(plane=plane,
                 start_page=0,
                 path=image,
                 go=not verify,
                 set_GPNVM=not boot_rom)
    print(' done.')

    if verify:
        print('Verifying...')
        stdout.flush()

        with open(image, 'rb') as f:
            data = f.read()
        # padding needed as target blake implementation can currently only handle
        # blocks of lengths divisible by 64
        pad_len = 64 - len(data) % 64
        padded_data = data + b'\x00' * pad_len
        digest = Blake2s(padded_data).digest()
        digest_target = p.blake2s(0x80000, len(data) + pad_len)
        print(' done.')
        if digest == digest_target:
            p.go(ub32(data[4:8]), wait=False)
        else:
            print('Image and flash hashes do not match, aborting...')
            exit(1)

    if reset:
        p.reset()
    time.sleep(.1)
    print('')

"""
test_programmer()

By now, it's mainly a samba test but can be used for programming as well.
"""
def test_programmer(port='/dev/ttyACM0', image='sam3x8e/images/flash.bin', ramimage='sam3x8e/images/bootloader.bin'):
    s = test_samba(port, ramimage='sam3x8e/images/bootloader.bin', go=True)
    t = Programmer(s)
    # test py.loader <--UART--> sam3x8e.bootloader.loader
    assert t.read_word(0x20002000)%256 == t.read_byte(0x20002000) 
    t.flash(path=image)
