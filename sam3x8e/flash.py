#!/usr/bin/env python3
"""sam3x8e/flash.py

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
from py.sam3x8e import program


if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	## 1. Positional Argument
	parser.add_argument("image")
	## 2. Optional Arguments
	parser.add_argument('--bootloader', default="bootloader.bin")
	parser.add_argument('--port', default="/dev/ttyACM0")
	parser.add_argument('--plane', type=int, default=0)
	## 3. Flags
	parser.add_argument('--erase', action="store_true", default=True)  # False if not there
	parser.add_argument('--reset', action="store_true", default=True)  # False if not there
	parser.add_argument('--verify', action="store_true", default=True) # False if not there
	parser.add_argument('--boot-rom', action="store_true") # False if not there

	args = parser.parse_args()
	
	print('Selected port:', args.port)
	print('Selected image:', args.image)
	
	program(port=args.port, image=args.image, erase=args.erase, reset=args.reset,\
			verify=args.verify, bootloader_image=args.bootloader, plane=args.plane, boot_rom=args.boot_rom)
	
	
