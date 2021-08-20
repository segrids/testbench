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

import struct
import binascii
import numpy as np

"""
Format conversions between 8-, 16-, or 32 bit unsigned integer representation 
in different formats:

mnemonic  | meaning             | 32 bit example       | first byte from left
--------- | ------------------- | -------------------- | ------------------
h / Hex   | ASCII / hex string  | 'deadbeef'           | most significant
b / Bin   | binary              | b'\xef\xbe\xad\xde'  | least significant 
u / uint* | unsigned int        | 0xdeadbeef           | most significant
v / Uint* | unsigned int vector | [0xef,0xbe,0xad,0xde]| least significant

"""

def vb32(b):
    """Convert a binary string into list of 32 bit integers.

    Example:
            vb32(b'\x00\x11\x22\x33\x44\x55\x66\x77') == [0x33221100,0x77665544]

    """
    if len(b) % 4 != 0:
        raise ValueError('Length of binary string has to be a multiple of 4.')
    return [ub32(b[i:i + 4]) for i in range(0, len(b), 4)]


def u32(x):
    """Limit an integer such that it fits within 32 bits.

    Example:
            u32(0x1234567890) == 0x34567890
    """
    return x & 0xFFFFFFFF

def ub8(b):
    """Convert bytes to an 8-bit integer.

    Example:
            ub8(b'\x12') == 0x12
    """
    return b[0]


def bu8(u):
    """Convert an 8-bit integer to bytes.

    Example:
            bu8(0x12) == b'\x12'
    """
    return bytes([u])

def ub16(b):
    """Convert bytes (LSB first) to a 16-bit integer.

    Example:
            ub16(b'\x12\x34') == 0x3412
    """
    (value, ) = struct.unpack('<H', b)
    return value


def bu16(value):
    """Convert a 16-bit integer to bytes (LSB first).

    Example:
            bu16(0x1234) == b'\x34\x12'
    """
    return struct.pack('<H', value)


def ub32(b):
    """Convert bytes (LSB first) to a 32-bit integer.

    Example:
            ub32(b'\x12\x34\x56\x78') == 0x78563412
    """
    (value, ) = struct.unpack('<I', b)
    return value


def bu32(value):
    """Convert a 32-bit integer to bytes (LSB first).

    Example:
            bu32(0x12345678) == b'\x78\x56\x34\x12'
    """
    return struct.pack('<I', value)


def hu(u):
    """Convert an integer to a decimal string representation.

    Example:
            hu(0x12345) == '12345'
    """
    return '%x' % u


def uh(a):  #'12345' -> 0x12345
    """Convert a decimal string representation of a number to an integer.

    Example:
            uh('12345') == 0x12345
    """
    return int(a, 16)

def hb(b):
    """
    Example:
        hb(b'\x12\xde\x11\x22\x33\x44') == '12de11223344'
    """
    return binascii.hexlify(b).decode()

def bh(h):
    """
    Example:
        bh('44332211de12') == b'\x44\x33\x22\x22\xde\x12'
    """
    return binascii.unhexlify(h)

def uh(h):
    """Convert a hexadecimal string representation of a number to an integer.

    Example:
            uh('0xabcdef0') == 0xabcdef0
            uh('abcdef0') == 0xabcdef0
    """
    return int(h.replace('0x', '').replace(' ',''), 16)

def u8(value):
    """Convert a signed 8 bit integer to an unsigned 8 bit integer.

    Example:
            u8(-1) == 0xff
    """
    return value % 0x100

def Uint8ToBin(x):
    """
    Example:
        Uint8ToBin([1,2,3,4,5]) == b'\x01\x02\x03\x04\x05'
    """
    return np.array(x, 'uint8').tobytes()

def BinToUint8(b):
    """
    Example:
        np.all(BinToUint8(b'\x01\x02\x03\x04') == np.array([1,2,3,4], 'uint8'))
    """
    return np.frombuffer(b, 'B')

def BinTouint32(b, lsb_first=True):
    """
    Example:
        BinTouint32(b'\x01\x02\x03\x04') == 0x04030201
    """
    if lsb_first:
        return np.frombuffer(b, '<u4')[0]
    else:
        return np.frombuffer(b, '>u4')[0]

def BinToUint32(b, lsb_first=True):
    """
    Example
        np.all(BinToUint32(b'\x01\x02\x03\x04\x05\x08\x07\x08') == np.array([0x04030201, 0x08070805], 'uint32'))
    """
    if lsb_first:
        return np.frombuffer(b, 'u4')
    else:
        return np.frombuffer(b, '>u4')

def Uint8Touint32(x, lsb_first=True):
    """
    Example:
        Uint8Touint32([1,2,3,4]) == 0x04030201
    """
    return BinTouint32(Uint8ToBin(x), lsb_first)

def Uint8ToUint32(x, lsb_first=True):
    """
    Example:
        np.all(Uint8ToUint32([1,2,3,4,5,6,7,8]) == np.array([0x04030201, 0x08070605], 'uint32'))
    """
    return BinToUint32(Uint8ToBin(x), lsb_first)


def Uint32ToBin(z, lsb_first=True):
    """
    Example:
        Uint32ToBin([0x01020304, 0x05060708]) == b'\x04\x03\x02\x01\x08\x07\x06\x05'
    """
    if lsb_first:
        return  np.array(z, '<u4').tobytes()
    else:
        return  np.array(z, '>u4').tobytes()

def Uint64ToBin(z, lsb_first=True):
    """
    Example:
        Uint64ToBin([0x0102030405060708]) == b'\x08\x07\x06\x05\x04\x03\x02\x01'
    """
    if lsb_first:
        return  np.array(z, '<u8').tobytes()
    else:
        return  np.array(z, '>u8').tobytes()

def Uint32ToUint8(z, lsb_first=True):
    """
    Example:
        np.all(Uint32ToUint8([0x01020304, 0x05060708]) == np.array([4,3,2,1,8,7,6,5], 'uint8'))
    """
    return BinToUint8(Uint32ToBin(z, lsb_first))


def uint32ToUint8(z, lsb_first=True):
    """
    Example:
        np.all(uint32ToUint8(0x01020304) == np.array([4,3,2,1], 'uint8'))
    """
    return Uint32ToUint8([z], lsb_first)    

def Uint64ToUint8(z, lsb_first=True):
    """
    Example:
        np.all(Uint64ToUint8([0x01020304]) == np.array([4,3,2,1,0,0,0,0], 'uint8'))
    """
    return BinToUint8(Uint64ToBin(z, lsb_first))


def uint64ToUint8(z, lsb_first=True):
    """
    Example:
        np.all(uint64ToUint8(0x01020304) == np.array([4,3,2,1,0,0,0,0], 'uint8'))
    """
    return Uint64ToUint8([z], lsb_first)    

def HexToBin(h):
    """
    Example:
        HexToBin('affe') == b'\xaf\xfe'
    """
    return bh(h)

def BinToHex(b):
    """
    Example:
        BinToHex(b'\xaf\xfe') == 'affe'
    """
    return hb(b)

def HexToHex(h):
        hexe = h.replace(' ','')
        if (len(hexe) % 2 == 1):
                print ("Fehler: ungerade Anzahl an Nibbeln!")
                return
        num = len(hexe)//2
        ret = ''
        for i in range(num-1):
                ret = ret + hexe[2*i] + hexe[2*i+1] + ' '
        ret = ret + hexe[2*(num-1)] + hexe[2*(num-1)+1]
        return ret

def HexToUint8(h):
    """
    Example:
        np.all(HexToUint8('affe') == np.array([0xfe, 0xaf], 'uint8'))
    """
    return BinToUint8(HexToBin(h))

def Uint8ToHex(x):
    """
    Example:
        Uint8ToHex([1,2,3,4]) == '04030201'
    """
    return BinToHex(Uint8ToBin(x))



def leftrotate32(x,n):
    """
    Example:
        leftrotate32(0x80000000,2) == 2 
    """
    return ((x << n) | (x >> (32-n))) & 0xffffffff


def Uint8_leftrotate32(x,n):
    return uint32ToUint8(leftrotate32(Uint8Touint32(x),n))

def test():
    assert vb32(b'\x00\x11\x22\x33\x44\x55\x66\x77') == [0x33221100,0x77665544]
    assert u32(0x1234567890) == 0x34567890
    assert ub8(b'\x12') == 0x12
    assert bu8(0x12) == b'\x12'    
    assert ub16(b'\x12\x34') == 0x3412
    assert bu16(0x1234) == b'\x34\x12'
    assert ub32(b'\x12\x34\x56\x78') == 0x78563412
    assert bu32(0x12345678) == b'\x78\x56\x34\x12'    
    assert hu(0x12345) == '12345'
    assert hb(b'\x12\xde\x11\x22\x33\x44') == '12de11223344'
    assert bh('12de11223344') == b'\x12\xde\x11\x22\x33\x44'
    assert uh('0xabcdef0') == 0xabcdef0
    assert uh('abcdef0') == 0xabcdef0
    assert u8(-1) == 0xff
    assert Uint8ToBin([1,2,3,4,5]) == b'\x01\x02\x03\x04\x05'
    assert np.all(BinToUint8(b'\x01\x02\x03\x04') == np.array([1,2,3,4], 'uint8'))
    # function with lsb_first argument
    assert BinToUint32(b'\x01\x02\x03\x04', True) == 0x04030201
    assert BinToUint32(b'\x01\x02\x03\x04', False) == 0x01020304
    assert np.all(BinToUint32(b'\x01\x02\x03\x04\x05\x08\x07\x08', True) == np.array([0x04030201, 0x08070805], 'uint32'))
    assert np.all(BinToUint32(b'\x01\x02\x03\x04\x05\x06\x07\x08', False) == np.array([0x01020304, 0x05060708], 'uint32'))
    assert Uint8Touint32([1,2,3,4], True) == 0x04030201
    assert Uint8Touint32([1,2,3,4], False) == 0x01020304
    assert np.all(Uint8ToUint32([1,2,3,4,5,6,7,8], True) == np.array([0x04030201, 0x08070605], 'uint32'))
    assert np.all(Uint8ToUint32([1,2,3,4,5,6,7,8], False) == np.array([0x01020304, 0x05060708], 'uint32'))
    assert Uint32ToBin([0x01020304, 0x05060708], True) == b'\x04\x03\x02\x01\x08\x07\x06\x05'
    assert Uint32ToBin([0x01020304, 0x05060708], False) == b'\x01\x02\x03\x04\x05\x06\x07\x08'
    assert Uint64ToBin([0x0102030405060708], True) == b'\x08\x07\x06\x05\x04\x03\x02\x01'
    assert Uint64ToBin([0x0102030405060708], False) == b'\x01\x02\x03\x04\x05\x06\x07\x08'
    assert np.all(Uint32ToUint8([0x01020304, 0x05060708], True) == np.array([4,3,2,1,8,7,6,5], 'uint8'))
    assert np.all(Uint32ToUint8([0x01020304, 0x05060708], False) == np.array([1,2,3,4,5,6,7,8], 'uint8'))
    assert np.all(uint64ToUint8(0x01020304, True) == np.array([4,3,2,1,0,0,0,0], 'uint8'))
    assert np.all(Uint64ToUint8([0x01020304], False) == np.array([0,0,0,0,1,2,3,4], 'uint8'))
    assert HexToBin('affe') == b'\xaf\xfe'
    assert BinToHex(b'\xaf\xfe') == 'affe'
    assert np.all(HexToUint8('affe') == np.array([0xaf, 0xfe], 'uint8'))
    assert Uint8ToHex([1,2,3,4]) == '01020304'
    assert leftrotate32(0x80000000,2) == 2 



