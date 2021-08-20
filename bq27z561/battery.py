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

from time import sleep
from ..hexen import *
from ..uart import *
from ..adapter import *

"""
HDQ API for bq27z561

Note: when talking to the BQ27Z561 sample on the eval module,
the USB port shall be powered. Otherwise the MCU pulls down
the SDA line. Alternatively: remove zero Ohm resistors R17 and R20. 
"""


"""
checksum function used by TI fuel gauges
"""
def checksum(block):           
        return bu8(0xff - sum([byte for byte in block]) % 0x100)


"""
class battery_class()

API to a few bq27z651 commands.
"""
class battery_class():
    def __init__(self, adapter):
        self.adapter = adapter

    def get_voltage(self):
        a = self.adapter
        assert a.protocol == b'H'
        return ub16(a.read(2, slave_address=b'\x08'))/1000.0

    def get_temperature(self):
        a = self.adapter
        assert a.protocol == b'H'
        return "%2.1f°C" %(ub16(a.read(2, slave_address=b'\x28'))/10.0 - 273.15)

    def get_control_status(self):
        a = self.adapter
        a.write(b'\x00\x00', slave_address=b'\x00')
        sleep(0.001)
        return ub16(a.read(2, slave_address=b'\x00'))

    def block_write(self, cmd, block): # tut in authentiction
        a = self.adapter
        a.write(cmd, slave_address=b'\x3e')
        sleep(0.02)
        a.write(block, slave_address=b'\x40')
        sleep(0.02)
        a.write(checksum(cmd + block) + bu8(len(block) + 4), slave_address=b'\x60')

    def block_read(self, cmd):
        a = self.adapter
        a.write(cmd, slave_address=b'\x3e')
        sleep(0.01)
        ret = a.read(34, slave_address=b'\x3e')
        print("cmd OK?", ret[:2] == cmd)
        print("checksum OK?", a.read(1, slave_address=b'\x60') == checksum(ret))
        length = ub8(a.read(1, slave_address=b'\x61'))
        return ret[2:length-2]

    def get_device_type(self): # 0x1562
        return self.block_read(b'\x01\x00')

    def get_fw_version(self): 
        return self.block_read(b'\x02\x00')

    def get_device_name(self):  # b'sn27z562'
        return self.block_read(b'\x4a\x00')

    def auth(self, challenge):
        a = self.adapter
        assert a.protocol == b'H'
        assert len(challenge) == 32
        a.write(b'\x00'*2, slave_address=b'\x3e')
        sleep(0.02)
        a.write(challenge, slave_address=b'\x40')
        sleep(0.02)
        a.write(checksum(challenge) + b'\x24', slave_address=b'\x60', trigger=True)
        sleep(0.250)
        return a.read(32, slave_address=b'\x40')


def battery(port="/dev/ttyACM0"):
    s = Serial(port)
    a = Adapter(s, interface="HDQ")
    return battery_class(a)


"""
Check reference response from the BQ27Z561 on the evaluation module:
"""
def check_auth(bat):
    assert bat.auth(b'\x00'*32) == b'\xb4\xbe\x11)\x8ch\xb8\x13K\xf3\xbc@\x9fE\x989}\x9f\xe1\xab'
    assert bat.auth(b'\x01'*32) == b'\x13\xb9\xb7\xa8_\xebALs\xfb\x7f\xe0_\xd2z&=2DP'






"""
bq27z561 was delivered in unsealed mode. Found the info on the web page:
The default unseal and full access keys are the same for all the gauges: 
0x0414 and 0x3672 (unseal), 
0xFFFF and 0xFFFF (full access)

Changing from UNSEALED to FULL ACCESS is performed by using the AltManufacturerAccess()
command, by writing the first word of the Full Access Key to AltManufacturerAccess(), followed by the
second word of the Full Access Key to AltManufacturerAccess().
"""
def full_access(a):
    a.write(b'\x3e\xaa\xaa')
    sleep(0.01)
    a.write(b'\x3e\xaa\xaa')

def change_keys(a, unseal_key=b'\xaa\xaa\xaa\xaa', full_key=b'\xaa\xaa\xaa\xaa'):
    a.write(b'\x3e\x35\x00')    
    sleep(0.01)
    a.write(b'\x40' + unseal_key + full_key)
    sleep(0.01)
    a.write(b'\x60' + checksum(b'\x35\x00' + unseal_key + full_key) + b'\x0c')

def operation_status(a):
    print("A:", hex(ub16(control2(a, 0x54, 6)[2:4])))
    print("B:", hex(ub16(control2(a, 0x54, 6)[4:6])))

"""
bq27z561 write auth key in full access mode DOESNT WORK!!!

Manual: "Initiate" OperationStatusA()[SEC1,SEC0] = 0,1 => FULL Access 
"""
def write_auth_key(a, key):
    assert len(key) == 32
    block_write(a, b'\x37\x00', key)    
    sleep(0.25)
    return block_read(a, b'\x37\x00')
    

