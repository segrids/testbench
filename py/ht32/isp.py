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

from collections import namedtuple

from ..rpcserial import connect
from ..hexen import *
from ..crc16 import crc16citt

'''
The HT32 boots the ISP booloader, if Pin A9 is low.
'''

class ISP():
    """Communication with the HT32 In-System Programming bootloader.

    Attributes:
        serial: A Serial instance connected to the target device.
    """

    def __init__(self, serial_or_port):
        """Initialize ISP and connect to target.

        Args:
            serial: A Serial instance connected to the target device.
        """
        if isinstance(serial_or_port, str):
            self.serial = connect(serial_or_port)
        else:
            self.serial = serial_or_port

    def info(self):
        """Request information about the target device.

        Returns:
            A named tuple containing the parsed information.
        """
        self.serial.write(_make_command(0x03))
        return _parse_info(self.serial.read(21))

    def read(self, addr, size):
        """Read a block of the target's memory.

        Args:
            addr: The integer address from where to read.
            size: The integer size of the block.
        """
        data = bytearray()
        for offset in range(0, size, 64):
            data += self._read_block(addr + offset)
        return bytes(data[:size])

    def _read_block(self, addr):
        """Utility method to read a 64 byte long block of memory.

        Args:
            addr: The integer address from where to read.
        """
        self.serial.write(_make_command(0x01, 0x02, addr, addr + 63))
        res = self.serial.read(65)
        # assert self.serial.read(1) == ord('O')
        assert res[-1] == ord('O')
        return res[:-1]

    def flash(self, addr, data):
        """Write to the targets flash memory.

        Args:
            addr: The integer address to which to write.
            data: The bytes of data to write.
        """
        for offset in range(0, len(data), 52):
            self._flash(addr + offset, data[offset:offset+52])

    def _flash(self, addr, data):
        """Utility method to write up to 52 bytes of data to the target's flash.

        Args:
            addr: The integer address to which to write.
            data: The bytes of data with a length less than or equal to 52.
        """
        assert len(data) <= 52
        self.serial.write(_make_command(0x01, 0x01, addr, addr + len(data) - 1, payload=data))
        assert self.serial.read(1) == b'O'

    def erase(self):
        """Erase the targets flash memory."""
        self.serial.write(_make_command(0x00, 0x0a))
        assert self.serial.read(1) == b'O'

    def page_erase(self, start_addr, end_addr):
        """Erase pages (a 512 bytes) of the targets flash memory.
           The end_addr will be rounded up to a multi page offset.
        """
        self.serial.write(_make_command(0x00, 0x08, start_addr=start_addr, end_addr=end_addr))
        assert self.serial.read(1) == b'O'


    def reset(self, iap=False):
        """Reset the target.

        Args:
            iap: Optional boolean to enter In-Application Programming on boot.
        """
        self.serial.write(_make_command(0x04, int(iap)))

def _make_command(cmd, parameter=0, start_addr=0, end_addr=0, payload=b''):
    """Utility function to assembly a command request to the ISP bootloader.

    Args:
        cmd: An integer specifying the command.
        parameter: Optional integer parameter.
        start_addr: Optional integer start address.
        end_addr: Optional integer end address.
        payload: Optional bytes to be included as data. Length cannot exceed 52.

    Returns:
        The encoded 64 byte request.
    """
    if len(payload) < 52:
        payload += b'\x00' * (52 - len(payload))
    blob = bytearray(bu8(cmd) + bu8(parameter) + b'\x00\x00' + bu32(start_addr) + bu32(end_addr) + payload)
    crc = crc16citt(bytes(blob))
    blob[2:4] = crc[::-1]
    return bytes(blob)

def _parse_info(blob):
    """Utility function to parse the block of data returned by the ISP's `info` command.

    Args:
        blob: The block of data returned by the ISP bootloader.

    Returns:
        A named tuple containing the parsed information.
    """
    assert len(blob) == 21
    assert blob[-1] == ord('O')
    info = [ub32(blob[4 * i:4 * (i + 1)]) for i in range(5)]

    iap = bool(info[0] >> 28)
    version = (info[0] >> 16) & 0xfff
    chip_name = info[0] & 0xffff
    if chip_name == 0:
        chip_name = info[4]

    page_size = info[1] >> 16
    start_addr = info[1] & 0xffff
    page_num = info[2] >> 16
    pp_bit_num = info[2] & 0xffff

    version_start_addr = info[3]

    Info = namedtuple('Info', 'iap version page_size start_addr page_num pp_bit_num version_start_addr chip_name')

    return Info(iap, version, page_size, start_addr, page_num, pp_bit_num, version_start_addr, chip_name)
