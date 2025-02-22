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

from .. import apdu
from ..hexen import *


class SWDFaultError(Exception):
    pass


class ParityError(Exception):
    pass


class DebugInitError(Exception):
    pass


class ProtocolError(Exception):
    pass


def retry(func):
    def f(self, *args, retry=3, **kwargs):
        for _ in range(retry - 1):
            try:
                return func(self, *args, **kwargs)
            except (ProtocolError, SWDFaultError) as e:
                for _ in range(retry):
                    try:
                        self.line_reset()
                        self.read_dp(self.IDCODE, retry=retry - 1)
                        if isinstance(e, SWDFaultError):
                            if (self.read_dp(self.CTRL_STAT, retry=retry - 1) >> 4) & 0xa != 0:
                                self.write_dp(self.ABORT, 0xc, retry=retry - 1)
                                assert (self.read_dp(self.CTRL_STAT, retry=retry - 1) >> 4) & 0xa == 0
                    except (ProtocolError, SWDFaultError):
                        pass
                    else:
                        break
                else:
                    raise e
        return func(self, *args, **kwargs)
    return f


def parity(x):
    parity = False

    while x != 0:
        if x & 1:
            parity = not parity
        x = x >> 1

    return parity


def request(addr, debug_port=True, read=True):
    """Build a SWD debug or access port request."""
    byte = 0
    if not debug_port:
        byte |= 1 << 6
    if read:
        byte |= 1 << 5
    a = [0, 2, 1, 3][addr >> 2]
    byte |= a << 3
    byte |= int(parity(byte)) << 2
    byte |= 0x81
    return bytes([byte])

"""
class SWD()

Connect with a Serial-Wire Debug interface over a Debugger-bridge which
communicates via APDU commands.

"""
class SWD():
    IDCODE = 0x00 # read only
    ABORT = 0x00 # write only
    CTRL_STAT = 0x04
    SELECT = 0x08

    def __init__(self, s, swj_switch=True):
        self.apdu = apdu.Apdu(s)
        if swj_switch:
            self.swj_switch()

        # FIXME: fails if switch sequence not executed
        debug_en = self.read_dp(self.CTRL_STAT) & 0x20000000
        if not debug_en:
            self.write_dp(self.CTRL_STAT, 0x50000000)
            for _ in range(3):
                debug_en = self.read_dp(self.CTRL_STAT) & 0x20000000
                if debug_en:
                    break
            else:
                raise DebugInitError()

    def swj_switch(self):
        """Engage the selection mechanism to enable SWD instead of JTAG and enable AHB-AP.

        Read idcode as sanity check.
        """
        res, status = self.apdu.sendreceive(b'W', res_len=4, cla=b'S')
        assert ub32(res) == 0x2ba01477

    @retry
    def read_ap(self, addr):
        """Read an Access Port register."""
        return self.read(request(addr, debug_port=False, read=True))

    @retry
    def read_dp(self, addr):
        """Read a Debug Port register."""
        try:
            return self.read(request(addr, debug_port=True, read=True))
        except SWDFaultError as e:
            if addr == self.IDCODE or addr == self.CTRL_STAT:
                raise ProtocolError()
            raise e

    @retry
    def write_ap(self, addr, val):
        """Write to an Access Port register."""
        self.write(request(addr, debug_port=False, read=False), val)

    @retry
    def write_dp(self, addr, val):
        """Write to a Debug Port register."""
        self.write(request(addr, debug_port=True, read=False), val)

    def read(self, reg):
        """Internal method to read from a Debug or Access Port register.

        Use `read_dp` or `read_ap` instead.
        """
        res, status = self.apdu.sendreceive(b'0', reg, res_len=4, cla=b'S')
        if status == 9:
            return ub32(res)
        elif status == 4:
            raise SWDFaultError()
        elif status == 8:
            raise ParityError()
        elif status == 2:
            # TODO: WAIT handling
            assert status == 9
        else:
            raise ProtocolError()

    def line_reset(self):
        self.apdu.sendreceive(b'2',cla=b'S')

    def target_reset(self):
        '''
        Arduino Due pin 5 is used as a reset line to reset the SWD target.
        '''
        self.apdu.sendreceive(b'2',cla=b'R')

    def write(self, reg, val):
        """Internal method to write to a Debug or Access Port register.

        Use `write_dp` or `write_ap` instead.
        """
        while True:
            _, status = self.apdu.sendreceive(b'1', bu32(val) + reg, cla=b'S')

            if status == 1:
                raise SWDFaultError()
            elif status != 3:
                break

    def idcode(self):
        """Read the SW-DP identification. Should be 0x2ba01477 on a Cortex-M3."""
        return self.read_dp(self.IDCODE)


