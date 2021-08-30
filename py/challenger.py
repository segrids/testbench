"""
All rights reserved
"""

from .hexen import *
from . import uart
from .apdu import Apdu
from .loader import Loader

"""
class Challenger():

Extend methods of Tester class by authentication methods.
"""
class Challenger(Loader):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def read_id(self):
        res, status = self.apdu.sendreceive(cla=b'R', ins=b'I', res_len=24)
        assert status == 0x9000
        return res

    def response(self, challenge=b'affe'*4):
        res, status = self.apdu.sendreceive(cla=b'R', ins=b'C', data=challenge, res_len=16)
        assert status == 0x9000
        return res


