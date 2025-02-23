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

import sys, os
#from glob import glob
from time import sleep
import serial


"""
class Sam3xSerial()

Extend the methods of serial.Serial by
 - a target_reset method for sam3x8e or ht32f52 eval module
 - an erase method for sam3x8e

Baudrate fixed to 115200.
"""
class Sam3xSerial(serial.Serial):

	def __init__(self, port, mcu='', **kwargs):
		'''Initialize a serial object without opening it.'''
		super(Sam3xSerial, self).__init__(**kwargs)
		self.port = port
		self.baudrate = 115200
		self.exclusive = False # only one instance can be exclusive
		self.timeout = 2
		self.mcu = "sam3x8e"
		self.open(port)

	def open(self, port=''):
		if port == '':
			port = self.port
		super(Sam3xSerial, self).open()

	def sendreceive(self, data, res_len):
		super(Sam3xSerial, self).write(data)
		return super(Sam3xSerial, self).read(res_len)

	def target_reset(self):
		if not self.is_open:
			self.open()
		self.close()
		sleep(0.4)
		self.open()
		
	def erase(self):
		baudrate = self.baudrate
		self.close()
		self.baudrate = 1200
		self.open()
		sleep(0.1)
		self.close()
		self.baudrate = baudrate
		self.open

