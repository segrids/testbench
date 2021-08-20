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

import numpy as np
from .hexen import *


def XOR(u, v):
    return u32(u ^ v)


def RROT(u, v):
    return u32((u >> v) | (u << (32 - v)))


IV = [
    0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A, 0x510E527F, 0x9B05688C,
    0x1F83D9AB, 0x5BE0CD19
]

sigma = [[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
         [14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3],
         [11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4],
         [7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8],
         [9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13],
         [2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9],
         [12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11],
         [13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10],
         [6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5],
         [10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0]]

abcd = [[0, 4, 8, 12], [1, 5, 9, 13], [2, 6, 10, 14], [3, 7, 11, 15],
        [0, 5, 10, 15], [1, 6, 11, 12], [2, 7, 8, 13], [3, 4, 9, 14]]


def block2list(block):
    assert len(block) == 64
    return [ub32(block[4 * i:4 * i + 4]) for i in range(16)]


"""
class Blake2s()

Implement the blake2s hash algorithm.
"""
class Blake2s():
    f0 = 0xffffffff  # set to ff...ff if block processed is the last, 00...00 otherwise

    def __init__(self, message):
        self.h = IV.copy()
        self.h[0] ^= 0x01010000 ^ 32
        self.t = [
            0, 0
        ]  # [low 32 bit word of counter, high 32 bit word of counter]
        self.ll = len(message)
        if self.ll % 64 == 0:
            self.message = message
        else:
            padlen = 64 - self.ll % 64
            self.message = message + b'\x00' * padlen
        # print('self.message', self.message)
        blockcount = len(self.message) // 64
        for i in range(blockcount - 1):
            m = block2list(self.message[64 * i:64 * i + 64])
            self.t[0] += 64
            self.F(m, False)
            # print('i=', i)
            # print(self.v)
        m = block2list(self.message[-64:])
        self.t[0] = self.ll
        self.F(m, True)

    def F(self, m, last):
        # initialize state
        self.v = self.h + IV
        self.v[12] ^= self.t[0]
        #self.v[13] ^= self.t[1]
        if last:
            self.v[14] ^= 0xffffffff
        for i in range(10):
            # print('i=', i)
            # print([hex(x) for x in self.v])
            self.G(0, 4, 8, 12, m[sigma[i][0]], m[sigma[i][1]])
            self.G(1, 5, 9, 13, m[sigma[i][2]], m[sigma[i][3]])
            self.G(2, 6, 10, 14, m[sigma[i][4]], m[sigma[i][5]])
            self.G(3, 7, 11, 15, m[sigma[i][6]], m[sigma[i][7]])
            self.G(0, 5, 10, 15, m[sigma[i][8]], m[sigma[i][9]])
            self.G(1, 6, 11, 12, m[sigma[i][10]], m[sigma[i][11]])
            self.G(2, 7, 8, 13, m[sigma[i][12]], m[sigma[i][13]])
            self.G(3, 4, 9, 14, m[sigma[i][14]], m[sigma[i][15]])

        for i in range(8):
            self.h[i] ^= self.v[i] ^ self.v[i + 8]

    def G(self, a, b, c, d, x, y):
        v = self.v
        v[a] = u32(v[a] + v[b] + x)
        v[d] = u32(RROT(XOR(v[d], v[a]), 16))
        v[c] = u32(v[c] + v[d])
        v[b] = u32(RROT(XOR(v[b], v[c]), 12))
        v[a] = u32(v[a] + v[b] + y)
        v[d] = u32(RROT(XOR(v[d], v[a]), 8))
        v[c] = u32(v[c] + v[d])
        v[b] = u32(RROT(XOR(v[b], v[c]), 7))

    def digest(self):
        return b''.join(bu32(x) for x in self.h)

    def hexdigest(self):
        return self.digest().hex()
