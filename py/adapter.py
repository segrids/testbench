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

from .apdu import Apdu
from .hexen import *

"""
An adapter is a device (e.g. Arduino Due or HT32F52) 
with a **slave interface** connected to a serial port of the test PC 
and a **master interface** connected to a port of the target (SAM3x8e, HT32F52, LPC8N04, battery ...)
"""

protocols = {"SPI": b'S', "I2C": b'I', "UART": b'U', "USART": b'V', "NFC": b'N', "HDQ": b'H'}

"""
class Adapter()

Provide methods open(), close(), write(), read(), flush(), reset()
in order to be able to hand it as argument to a higher layer object (in particular an apdu.Apdu)
alternatively to a Serial object.
"""
class Adapter():

    def __init__(self, *args, interface="I2C", slave_address=0x11, config=4, **kwargs):
        """Initiate Debugger and configure target's debug port.

        Args:
            interface: master interface of the adapter connected to the end target
            slave_address: slave address or slave select line of the end target if applicable
            config: configuration of the master interface of the adapter
            *args, **kwargs: passed to base class `Target`.
        """
        self.apdu = Apdu(*args, **kwargs)
        self.protocol = protocols[interface]
        self.slave_address = bu8(slave_address)
        self.config = bu8(config)
        self.mcu = None # not neccessary since Adapter has mcu agnostic target_reset method
        self.open()

    def open(self):
        res, status = self.apdu.sendreceive(cla=b'A', ins=b'O', data=self.protocol+self.config)
        assert status == 0x9000

    def close(self):
        res, status = self.apdu.sendreceive(cla=b'A', ins=b'C')
        assert status == 0x9000

    def flush(self):
        res, status = self.apdu.sendreceive(cla=b'A', ins=b'F')
        assert status == 0x9000

    def target_reset(self): 
        res, status = self.apdu.sendreceive(cla=b'A', ins=b'T')
        assert status == 0x9000

    def self_reset(self):
        self.apdu.serial.target_reset()

    def read(self, length, slave_address=b''):
        if slave_address == b'':
            slave_address = self.slave_address
        res, status = self.apdu.sendreceive(cla=b'A', ins=b'R', data=slave_address, res_len=length)
        assert status == 0x9000
        return res

    def write(self, data, slave_address=b'', trigger=False):
        if slave_address == b'':
            slave_address = self.slave_address
        assert len(slave_address) == 1
        if trigger:
            ins = b'w'
        else:
            ins = b'W'
        res, status = self.apdu.sendreceive(cla=b'A', ins=ins, data=slave_address+data)
        assert status == 0x9000

    def sendreceive(self, data, res_len, slave_address=b'', trigger=False):
        if slave_address == b'':
            slave_address = self.slave_address
        assert len(slave_address) == 1
        if trigger:
            ins = b'x'
        else:
            ins = b'X'
        res, status = self.apdu.sendreceive(cla=b'A', ins=ins, data=slave_address+data, res_len=res_len)
        assert status == 0x9000
        return res
		

"""
test_adapter()

Required a programmed sam3x8e device at the adapter_port
connected to a second progarmmed target connected via `interface`.

The interface select jumper must be be set properly
(see sam3x8e/adapter.c)

If interface == "SPI", connect arduiono due pin A10 of the target (i.e. the SPI slave) to GND.
"""
def test_adapter(adapter_port="/dev/ttyACM2", interface="I2C"):
    from .sam3xserial import Sam3xSerial
    from .loader import Loader
    s = Sam3xSerial(adapter_port)
    a = Adapter(s, interface=interface)
    t = Loader(a)
    assert t.echo(b'abc') == b'abc'

