#!/usr/bin/env python3
"""ht32/flash.py

Usage:
  flash.py [<image>] [options]
  flash.py (-h | --help)

Options:
  -h --help		  Show this screen.
  --erase			Erase the target before flashing.
  --port=<p>		 Target device port [default: ttyACM0].
  -v --verify		Hash the flash and compare to binary.
  -r --reset		 Reset the CPU (after write).
  --bootloader=<bl>  Specify a custom bootloader binary
					 [default: sam3x8e/bootloader.bin].
  --plane=<pl>	   Select flash plane 0 or 1 [default: 0].
  --boot-rom		 Boot from ROM.
"""
from sys import exit, stdout
import time
import argparse

import py
from py.ht32.isp import ISP
#from py.ht32.ht32serial import Ht32Serial

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	## 1. Positional Argument
	parser.add_argument("image")
	## 2. Optional Arguments
	#parser.add_argument('--port', default="/dev/ttyUSB0")
	parser.add_argument('--adapter', default="/dev/ttyACM0")

	args = parser.parse_args()
	
	print('Selected adapter port:', args.adapter)
	print('Selected image:', args.image)
	# Open the Adapter
	s = py.Sam3xSerial(args.adapter)
	# Programming only works over the UART interface
	a = py.adapter.Adapter(s, interface="USART")

	with open(args.image, 'rb') as f:
		binary = f.read()
	isp = ISP(a)
	time.sleep(0.3)
	print(isp.info())
	isp.page_erase(start_addr=0x0, end_addr=0x1000)
	time.sleep(0.3)
	isp.flash(0x00, binary)
	isp.reset()

