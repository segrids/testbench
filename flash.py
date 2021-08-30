#!/usr/bin/env python3
"""flash.py

Usage:
  flash.py [<image>] [options]
  flash.py (-h | --help)

Options:
  -h --help          Show this screen.
  --target=<target>  Select the target device [default: SAM3x8e].
  --erase            Erase the target before flashing.
  --port=<p>         Target device port [default: ttyACM0].
  -v --verify        Hash the flash and compare to binary.
  -r --reset         Reset the CPU (after write).
  --bootloader=<bl>  Specify a custom bootloader binary
                     [default: sam3x8e/bootloader.bin].
  --plane=<pl>       Select flash plane 0 or 1 [default: 0].
  --boot-rom         Boot from ROM.
"""
from sys import exit, stdout
import time

from docopt import docopt
from py.uart import Serial

from py import *

if __name__ == '__main__':
    args = docopt(__doc__)
    target = args['--target']
    image = args['<image>']
    port = '/dev/' + args['--port']
    plane = int(args['--plane'])
    bootloader = args['--bootloader']
    verify = args['--verify']
    erase = args['--erase']
    boot_rom = args['--boot-rom']
    reset = args['--reset']

    print('Selected port:', port)
    print('Selected image:', image)

    if target == 'HT32':
        from py.ht32.isp import ISP, isp
        image = args['<image>']
        if image is None:
            if reset:
                isp(Serial(port)).reset()
            else:
                print('No image specified, not flashing.')
        else:
            with open(image, 'rb') as f:
                binary = f.read()
            isp = ISP(Serial(port))
            isp.page_erase(start_addr=0x0, end_addr=0x1000)
            isp.flash(0x00, binary)
            isp.reset()

    elif target == 'SAM3x8e':
        from py.sam3x8e.programmer import program
        if bootloader is None:
            bootloader = 'sam3x8e/bootloader.bin'
        program(port, image=image, erase=erase, reset=True,\
                verify=verify, bootloader_image=bootloader, plane=plane, boot_rom=boot_rom)
    else:
        print('Unknown target.')
