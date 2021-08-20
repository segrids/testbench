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
from py.sam3x8e.sam3x8e import Sam3x8e
from py.sam3x8e.bootloader import *
#from py.ht32.isp import *

if __name__ == '__main__':
    args = docopt(__doc__)
    target = args['--target']
    port = '/dev/' + args['--port']
    plane = int(args['--plane'])
    verify = args['--verify']
    boot_rom = args['--boot-rom']
    reset = args['--reset']

    print('Selected device:', port)

#    if target == 'HT32':
#        image = args['<image>']
#        if image is None:
#            if reset:
#                ###ISP(Serial(port)).reset()
#                isp(Serial(port)).reset()
#            else:
#                print('No image specified, not flashing.')
#        else:
#            with open(image, 'rb') as f:
#                binary = f.read()
#            isp = ISP(Serial(port))
#            isp.page_erase(start_addr=0x0, end_addr=0x1000)
#            isp.flash(0x00, binary)
#            isp.reset()
#    elif target == 'SAM3x8e':
    if target == 'SAM3x8e':
        if args['--erase']:
            print('Erasing...', end='')
            stdout.flush()
            erase(port)
            print(' done.')

        image = args['<image>']
        if image is None:
            print('No image specified, not flashing.')
        else:
            print('Initializing connection...', end='')
            stdout.flush()
            # connect to boot code
            rl = bootLoader(port)
            print(' done.')

            print('Uploading bootloader...', end='')
            stdout.flush()
            # load and run bootloader
            rl.load(0x20071000, args['--bootloader'], go=True)
            print(' done.')

            print('Connecting to bootloader...', end='')
            stdout.flush()
            # connect to bootloader
            assert isinstance(rl.serial, uart.Serial)
            target = Sam3x8e(rl.serial)
            print(' done.')
            time.sleep(0.1)

            print('Flashing...', end='')
            stdout.flush()
            target.flash(plane=plane,
                         start_page=0,
                         path=image,
                         go=not verify,
                         set_GPNVM=not boot_rom)
            print(' done.')

            if verify:
                print('Verifying...', end='')
                stdout.flush()

                with open(image, 'rb') as f:
                    data = f.read()
                # padding needed as target blake implementation can currently only handle
                # blocks of lengths divisible by 64
                pad_len = 64 - len(data) % 64
                padded_data = data + b'\x00' * pad_len
                digest = Blake2s(padded_data).digest()
                digest_target = target.blake2s(0x80000, len(data) + pad_len)
                print(' done.')
                if digest == digest_target:
                    target.go(ub32(data[4:8]), wait=False)
                else:
                    print('Image and flash hashes do not match, aborting...')
                    exit(1)

            if reset:
                target.reset()
            time.sleep(.1)
    else:
        print('Unknown target.')
