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
from . import uart
from .hexen import *


class Timeout(Exception):
    pass

"""
apdu()

a wrapper for the Apdu class
"""
def apdu(port_or_slot, remote=False):
    if remote:
        serial = rpcserial.SerialClient()
        serial.open(port_or_slot)
    else:
        serial = rpcserial.Serial(port_or_slot)
    return Apdu(serial)

"""
class Apdu()

Apdu package handler
"""
class Apdu():

    def __init__(self, serial):
        self.serial = serial

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.disconnect()

    def connect(self):
        self.serial.open()

    def disconnect(self):
        self.serial.close()

    def clear_buffer(self):
        self.serial.read_all()

    def sendreceive(self, ins, data=b'', res_len=0, cla=b'B', read_response=True):
        """Issue an APDU command.

        Args:
            ins: A single byte (bytes of length 1) specifying the command.
            data: Optional bytes to be sent as the body of the request. Request
                body left empty by default.
            res_len: Optional integer number of bytes expected as the response
                body (thus excluding the 16-bit status code).
            read_response: Optional boolean to specify whether to wait for and
                parse the response or return immediately.

        Returns:
            A tuple containing the 16-bit status code and the bytes of the
            response body or None if `read_response` is false.
        """
        cmd = cla + ins + bu16(len(data)) + data + bu16(res_len)
        #print("sending APDU", cmd) # uncomment for debugging
        self.serial.write(cmd)
        sleep(0.001) ### added for adapter class
        if read_response and res_len is not None:
            res = self.serial.read(res_len + 2)

            if len(res) != res_len + 2:
                raise Timeout(
                    'Received {} bytes in {} seconds but expected {}.'.format(
                        len(res), self.serial.timeout, res_len + 2))
            return res[:-2], ub16(res[-2:])

