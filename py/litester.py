
import serial
from .apdu import *
from .li import *
from .hexen import *
from .loader import Loader

"""
Example: Start from folder `~/pakete`

> import ignite
> t = ignite.Ignite("/dev/ttyUSB0")
> t.test_echo()
> hex(t.read_word(0x0))

"""

def litester(port="/dev/ttyACM0"):
	from .sam3x8a.sam3xserial import Sam3xSerial
	serial = Sam3xSerial(port)
	return Litester(serial)


"""
class Litester():

Extend methods of Loader class by long integer methods.
"""
class Litester(Loader):

	def __init__(self, *args, **kwargs):
		super().__init__(*args, **kwargs)

	"""
		x: Integer, max 1024-bit 
	"""
	def write_xdn(self, x, d, n):
		xli = int_to_li(x, 32)
		data = Uint32ToBin(xli)
		ret = self.apdu.sendreceive(cla=b"I", ins=b"X", data=data, res_len=0)
		if int(ret[1]) != 0x9000:
			print("ERROR: ret =", ret)
		else:
			print("x=%d geschrieben" %x)
		dli = int_to_li(d, 32)
		data = Uint32ToBin(dli)
		ret = self.apdu.sendreceive(cla=b"I", ins=b"D", data=data, res_len=0)
		if int(ret[1]) != 0x9000:
			print("ERROR: ret =", ret)
		else:
			print("d=%d geschrieben" %d)
		nli = int_to_li(n, 32)
		data = Uint32ToBin(nli)
		ret = self.apdu.sendreceive(cla=b"I", ins=b"N", data=data, res_len=0)
		if int(ret[1]) != 0x9000:
			print("ERROR: ret =", ret)
		else:
			print("n=%d geschrieben" %n)

	def write_y(self, y):
		yli = int_to_li(y, 32)
		data = Uint32ToBin(yli)
		ret = self.apdu.sendreceive(cla=b"I", ins=b"Y", data=data, res_len=0)
		if int(ret[1]) != 0x9000:
			print("ERROR: ret =", ret)
		else:
			print("y=%d geschrieben" %y)

	def read_xdn(self):
		ret = self.apdu.sendreceive(cla=b"I", ins=b"x", res_len=128)
		if int(ret[1]) != 0x9000:
			print("FEHLER beim Lesen von x: ret =", ret)
		else:
			xli = BinToUint32(ret[0])
			x = li_to_int(xli)	
			print("x=%d gelesen" %x)
		ret = self.apdu.sendreceive(cla=b"I", ins=b"d", res_len=128)
		if int(ret[1]) != 0x9000:
			print("FEHLER beim Lesen von d: ret =", ret)
		else:
			dli = BinToUint32(ret[0])
			d = li_to_int(dli)	
			print("d=%d gelesen" %d)
		ret = self.apdu.sendreceive(cla=b"I", ins=b"n", res_len=128)
		if int(ret[1]) != 0x9000:
			print("FEHLER beim Lesen von n: ret =", ret)
		else:
			nli = BinToUint32(ret[0])
			n = li_to_int(nli)	
			print("n=%d gelesen" %n)
		return x,d,n

	def read_y(self):
		ret = self.apdu.sendreceive(cla=b"I", ins=b"y", res_len=128)
		if int(ret[1]) != 0x9000:
			print("FEHLER beim Lesen von y: ret =", ret)
		else:
			yli = BinToUint32(ret[0])
			y = li_to_int(yli)
		return y

	def read_buf(self):
		ret = self.apdu.sendreceive(cla=b"I", ins=b"b", res_len=256)
		if int(ret[1]) != 0x9000:
			print("FEHLER beim Lesen von buf: ret =", ret)
		else:
			bufli = BinToUint32(ret[0])
			buf = li_to_int(bufli)
		return buf

	def write_buf(self, buf):
		bufli = int_to_li(buf, 64)
		data = Uint32ToBin(bufli)
		ret = self.apdu.sendreceive(cla=b"I", ins=b"B", data=data[:128], res_len=0)
		ret = self.apdu.sendreceive(cla=b"I", ins=b"H", data=data[128:], res_len=0)
		if int(ret[1]) != 0x9000:
			print("ERROR: ret =", ret)
		else:
			print("buf=%d geschrieben" %buf)

	def get_bitlen(self):
		ret = t.apdu.sendreceive(cla=b"I", ins=b'l', res_len=20)
		if int(ret[1]) != 0x9000:
			print("FEHLER beim Lesen von buf: ret =", ret)
		else:
			ret = BinToUint32(ret[0]) # [bitlen(y), bitlen(x), bitlen(d), bitlen(n), bitlen(buf)]	

	def get_n_word_shifted(self, word, shift):
		ret = self.apdu.sendreceive(cla=b"I", ins=b's', data=Uint32ToBin([word,shift]), res_len=4)
		assert ret[1] == 0x9000
		return ub32(ret[0])	

	def test_get_word_shifted(self, n):
		self.write_xdn(0,0,n)
		assert self.read_xdn()[2] == n
		print("n erfolgreich geschrieben")
		for shift in range(100):
			for word in range(64):
				res = self.get_n_word_shifted(word, shift)
				assert res == int_to_li(n<<shift,64)[word]
				print("check OK for shift=%d, word=%d" %(shift, word))

	def test_add_shifted(self, y, x, shift):
		self.write_xdn(x,0,0)
		assert self.read_xdn()[0] == x
		print("x erfolgreich geschrieben")
		self.write_y(y)
		assert self.read_y() == y
		print("y erfolgreich geschrieben")
		ret = self.apdu.sendreceive(cla=b"I", ins=b'a', data=bu32(shift), res_len=0)
		assert ret[1] == 0x9000
		buf = self.read_buf()
		assert buf == y + (x << shift)	

	def test_sub_shifted(self, y, x, shift):
		assert y >= x
		self.write_xdn(x,0,0)
		assert self.read_xdn()[0] == x
		print("x erfolgreich geschrieben")
		self.write_y(y)
		assert self.read_y() == y
		print("y erfolgreich geschrieben")
		ret = self.apdu.sendreceive(cla=b"I", ins=b'm', data=bu32(shift), res_len=0)
		assert ret[1] == 0x9000
		buf = self.read_buf()
		assert buf == y - (x << shift)	

	def test_geq_shifted(self, buf, n, shift):
		assert buf >=n
		checker = buf % n
		self.write_buf(buf)
		assert self.read_buf() == buf
		print("buf erfolgreich geschrieben")
		self.write_xdn(0,0,n)
		assert self.read_xdn()[2] == n
		print("n erfolgreich geschrieben")
		ret = self.apdu.sendreceive(cla=b"I", ins=b'k', data=bu32(shift), res_len=4)
		assert ret[1] == 0x9000
		print (ub32(ret[0]) == 1)
		assert ub32(ret[0]) == 1*(buf >= (n<<shift))

	"""
		buf = buf % n
	"""
	def test_mod(self, buf, n):
		checker = buf % n
		self.write_buf(buf)
		assert self.read_buf() == buf
		print("buf erfolgreich geschrieben")
		self.write_xdn(0,0,n)
		assert self.read_xdn()[2] == n
		print("n erfolgreich geschrieben")
		ret = self.apdu.sendreceive(cla=b"I", ins=b'q', data=b'', res_len=0)
		assert ret[1] == 0x9000
		buf = self.read_buf()
		print(hex(buf))
		assert buf == checker
	
	"""
		buf = x*y
	
	Dauert bei zwei 1024bit-Zahlen ca 50s.
	Dauert bei zwei  512bit-Zahlen ca 25s (1024bit-Implementierung).
	
	"""
	def test_mul(self, x, y):
		self.write_xdn(x,0,0)
		assert self.read_xdn()[0] == x
		print("x erfolgreich geschrieben")
		self.write_y(y)
		assert self.read_y() == y
		print("y erfolgreich geschrieben")
		ret = self.apdu.sendreceive(cla=b"I", ins=b'*', data=b'', res_len=0)
		assert ret[1] == 0x9000
		buf = self.read_buf()
		print(hex(buf))
		assert buf == x*y

	def test_modexp(self, x, d, n):
		x = x % n
		d = d % n
		self.write_xdn(x,d,n)
		assert self.read_xdn()[0] == x
		assert self.read_xdn()[1] == d
		assert self.read_xdn()[2] == n
		print("x,d,n erfolgreich geschrieben")
		self.write_y(0)
		wordlen = len(int_to_li(n))
		ret = self.apdu.sendreceive(cla=b"I", ins=b'p', data=bu32(wordlen), res_len=0)
		assert ret[1] == 0x9000
		y = self.read_y()
		print(hex(y))
		assert y == pow(x,d,n)

