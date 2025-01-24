# li-Klasse
# Langzahlen als arrays von 32-bit Integern
# je kleiner der Index desto kleiner die Wertigkeit

import numpy as np

def uint32_leftshift(x,n):
	return np.uint32(np.left_shift(x,n) % 0x1_0000_0000)

def uint32_rightshift(x,n):
	return np.uint32(np.right_shift(x,n) % 0x1_0000_0000)

def li_print(a):
	print(hex(li_to_int(a)), "(len=%d)" %len(a))

def li_to_int(a):
	ret = 0
	arity = 1
	for i in range(len(a)):
		ret += int(a[i]) * arity
		arity *= 0x1_0000_0000
	return ret

def test_li_to_int():
	assert hex(li_to_int([0xaaaabbbb,0xccccdddd,0xeeeeffff])) == '0xeeeeffffccccddddaaaabbbb'

def int_to_li(x, l=None):
	checker = x ##
	ret = [np.uint32(x % 0x1_0000_0000)]
	x = x // 0x1_0000_0000
	while x != 0:
		ret += [np.uint32(x % 0x1_0000_0000)]
		x = x // 0x1_0000_0000
	if l is not None:
		ret += [np.uint32(0)]*(l-len(ret))
	assert li_to_int(ret) == checker ##
	return ret

def test_int_to_li(N):
	assert N == li_to_int(int_to_li(N))

def uint32_bitlen(x):
	ret = 0
	while x != 0:
		x = x//2
		ret = ret + 1
	return ret

def li_bitlen(a):
	N = len(a)
	for i in range(N-1,-1,-1):
		if a[i] != 0:
			return (i*32) + uint32_bitlen(a[i])
	return 0

def test_li_bitlen(n):
	assert li_bitlen(int_to_li(1<<n)) == n+1

def li_clear(a):
	for i in range(len(a)):
		a[i] = np.uint32(0)
	return

# c braucht nicht leer zu sein
def li_copy(c,a):
	for i in range(min(len(a),len(c))):
		c[i] = a[i]
	for i in range(len(a), len(c)):
		c[i] = np.uint32(0)
	assert li_to_int(c) == li_to_int(a)
	return

def li_geq(a,b):
	assert len(b) == len(a)
	for i in range(len(a)-1, -1, -1):
		if a[i] < b[i]:
			assert li_to_int(a) < li_to_int(b)
			return False
		elif a[i] > b[i]:
			assert li_to_int(a) > li_to_int(b)
			return True
	assert li_to_int(a) == li_to_int(b)
	return True

def li_getbit(a, n):
	return np.bitwise_and(uint32_rightshift(a[n//32], n%32), 1) 

def test_li_getbit(A):
	a = int_to_li(A)
	checker = np.binary_repr(A)[::-1]
	if A != 0:
		assert len(checker) == li_bitlen(a)
	for n in range(len(checker)):
		assert li_getbit(a,n) == int(checker[n])
	for n in range(len(checker), len(a)//32):
		assert li_getbit(a,n) == 0
	return

def li_get_word_shifted(b,i,shift):
	assert i >= 0
	if i < shift//32:
		return np.uint32(0)
	elif i == shift//32:
		return uint32_leftshift(b[0], shift%32)
	elif i < len(b) + shift//32:
		return np.uint32(uint32_leftshift(b[i-shift//32], shift%32) +\
                       uint32_rightshift(b[i-shift//32-1], 32-shift%32))
	elif i == len(b) + shift//32:
		return uint32_rightshift(b[len(b)-1], 32-shift%32)
	if i > len(b) + shift//32:
		return np.uint32(0)

def test_li_get_word_shifted():
	b = np.uint32([0x7fff_ffff,1])
	assert li_get_word_shifted(b,1,0) == 1
	assert li_get_word_shifted(b,1,1) == 2
	assert li_get_word_shifted(b,1,2) == 5
	assert li_get_word_shifted(b,1,3) == 11
	assert li_get_word_shifted(b,1,32) == 0x7fff_ffff
	b = np.uint32([0xffff_ffff,1])
	assert li_get_word_shifted(b,1,1) == 3

def li_geq_shifted(a,b,shift):
	assert len(b) + shift//32 <= len(a)
	for i in range(len(a)-1, -1, -1):
		shifted_bi = li_get_word_shifted(b,i,shift)
		if a[i] < shifted_bi:
			assert li_to_int(a) < li_to_int(b)<<shift
			return False
		elif a[i] > shifted_bi:
			assert li_to_int(a) > li_to_int(b)<<shift
			return True
	assert li_to_int(a) == li_to_int(b)<<shift
	return True

def test_li_geq_shifted(A,B,shift):
	li_geq_shifted(int_to_li(A),int_to_li(A>>shift),shift)
	li_geq_shifted(int_to_li(A<<shift),int_to_li(A),shift)
	li_geq_shifted(int_to_li(A<<shift + 0xff),int_to_li(A),shift)
	li_geq_shifted(int_to_li(A),int_to_li(B),len(int_to_li(A)))
	li_geq_shifted(int_to_li(A), int_to_li(B), shift)


# Der Aufrufer muss dafür sorgen, dass len(c) so gross ist, dass das Ergebnis
# genau stimmt.
def li_add(c,a):
	checker = li_to_int(c) + li_to_int(a)
	assert len(c) >= len(a)
	carry = np.uint64(0)
	for i in range(len(a)):
		s = np.uint64(c[i]) + np.uint64(a[i]) + carry
		c[i] = np.uint32(s % 0x1_0000_0000)
		carry = s // 0x1_0000_0000
	for i in range(len(a), len(c)):
		s = np.uint64(c[i]) + carry
		c[i] = np.uint32(s % 0x1_0000_0000)
		carry = s // 0x1_0000_0000
	assert li_to_int(c) == checker
	return

def test_li_add(C,A):
	Cli = int_to_li(C)
	Ali = int_to_li(A)
	li_add(Cli, Ali)

# Der Aufrufer muss dafür sorgen, dass len(c) so gross ist, dass das Ergebnis
# genau stimmt.
def li_add_shifted(c,a,shift):
	checker = li_to_int(c) + (li_to_int(a)<<shift)
	assert len(c) >= len(a) + shift//32
	carry = np.uint64(0)
	for i in range(len(c)):
		s = np.uint64(c[i]) + li_get_word_shifted(a,i,shift) + carry
		c[i] = np.uint32(s % 0x1_0000_0000)
		carry = s // 0x1_0000_0000
	assert li_to_int(c) == checker
	return

def test_li_add_shifted(C,A,shift):
	li_add_shifted(int_to_li(C), int_to_li(A), shift)

# Der Aufrufer muss für c > a sorgen.
def li_sub(c,a):
	assert len(c) >= len(a)
	assert li_to_int(c) >= li_to_int(a)
	checker = li_to_int(c) - li_to_int(a)
	s = np.uint64(c[0]) + np.uint64(np.bitwise_not(a[0])) + np.uint64(1)
	c[0] = np.uint32(s % 0x1_0000_0000)
	carry = np.uint64(s // 0x1_0000_0000)
	for i in range(1,len(a)):
		s = np.uint64(c[i]) + np.uint64(np.bitwise_not(a[i])) + carry
		c[i] = np.uint32(s % 0x1_0000_0000)
		carry = np.uint64(s // 0x1_0000_0000)
	for i in range(len(a),len(c)):
		s = np.uint64(c[i]) + np.uint64(0xffffffff) + carry
		c[i] = np.uint32(s % 0x1_0000_0000)
		carry = np.uint64(s // 0x1_0000_0000)
	#assert carry == 1
	assert li_to_int(c) == checker
	return

def test_li_sub(C,A):
	A = A % C # will A < C
	Cli = int_to_li(C)
	Ali = int_to_li(A)
	li_sub(Cli, Ali)

def li_sub_shifted(c,a,shift):
	assert len(c) >= len(a)
	assert li_to_int(c) >= (li_to_int(a)<<shift)
	checker = li_to_int(c) - (li_to_int(a)<<shift)
	carry = np.uint64(1)
	for i in range(0,len(c)):
		c[i] = np.uint32((carry + c[i]) % 0x1_0000_0000)
		carry1 = (c[i] < carry)*1
		adder = li_get_word_shifted(a,i,shift)
		adder = np.uint64(np.bitwise_not(adder))
		c[i] = np.uint32((adder + c[i]) % 0x1_0000_0000)
		carry2 = (c[i] < adder)*1
		carry = np.uint64(carry1 + carry2)
		assert carry < 2
	assert li_to_int(c) == checker
	return

# Der Aufrufer muss für c > a sorgen.
def li_sub_shifted_org(c,a,shift):
	assert len(c) >= len(a)
	assert li_to_int(c) >= (li_to_int(a)<<shift)
	checker = li_to_int(c) - (li_to_int(a)<<shift)
	carry = np.uint64(1)
	for i in range(0,len(c)):
		shifted_ai = li_get_word_shifted(a,i,shift)
		s = np.uint64(c[i]) + np.uint64(np.bitwise_not(shifted_ai)) + carry
		c[i] = np.uint32(s % 0x1_0000_0000)
		carry = np.uint64(s // 0x1_0000_0000)
	assert li_to_int(c) == checker
	return

def test_li_sub_shifted(C,A):
	Ali = int_to_li(A)
	Cli = int_to_li(C)
	for shift in range(100):
		while C < A<<shift:
			C = C**2
		li_sub_shifted(int_to_li(C,len(Cli)+len(Ali)+shift//32), Ali, shift)


def li_fineleftshift(c, a, n):
	assert n < 32
	assert (li_bitlen(a) + n + 31) // 32 <= len(a)
	for i in range(len(c)-1, len(a)-1, -1):
		c[i] = np.uint32(0)
	for i in range(len(a)-1, 0,-1):
		c[i] = uint32_leftshift(a[i],n) + uint32_rightshift(a[i-1],32-n)
	c[0] = uint32_leftshift(a[0], n)
	return

def li_coarseleftshift(c,a,m):
	assert len(c) >= len(a) + m
	for i in range(len(c)-1, len(a)+m-1,-1):
		c[i] = np.uint32(0)
	for i in range(len(a)+m-1, m-1,-1):
		c[i] = a[i-m]
	for i in range(m-1,-1,-1):
		c[i] = np.uint32(0)
	return	

# c braucht nicht leer sein
# Der Aufrufer muss dafür sorgen, dass c gross genug ist.
def li_leftshift(c,a,n):
	li_coarseleftshift(c,a,n//32)
	li_fineleftshift(c,c,n%32)
	assert li_to_int(c) == li_to_int(a) * 2**n
	return

def test_li_leftshift(A,n):
	Ali = int_to_li(A)
	Cli = int_to_li(0xffffffff, len(Ali) + n//32+2)
	li_leftshift(Cli,Ali,n)
	assert li_to_int(Cli) == A * 2**n


# a = a%a
# in der Anwendung ist len(a) = 2*len(n)
# und während der Berechnung: len(c) == len(a)
# nach li_mod kann c auf len(n) gekürzt werden
def li_mod(a,n):
	checker = li_to_int(a) % li_to_int(n)
	assert len(a) >= len(n)
	d = li_bitlen(a) - li_bitlen(n)
	while d >= 0:
		if li_geq_shifted(a,n,d):
			li_sub_shifted(a,n,d)
		d = d-1
	assert li_to_int(a) == checker
	return

def test_li_mod(A,N):
	Ali = int_to_li(A)
	Nli = int_to_li(N)
	if len(Nli) < len(Ali):
		Nli = int_to_li(N,len(Ali))
	li_mod(Ali,Nli)

# c braucht nicht leer sein
# buf braucht nicht leer sein
# in der Anwendung ist len(a) = 2*len(n)
# und während der Berechnung: len(c) == len(a)
# nach li_mod kann c auf len(n) gekürzt werden
def li_mod_old(c,a,n,buf):
	assert len(c) >= len(n)
	assert len(c) >= len(a)
	assert len(buf) >= len(a)
	li_copy(c,a)
	d = li_bitlen(a) - li_bitlen(n)
	while d >= 0:
		li_leftshift(buf,n,d)
		if li_geq(c,buf):
			li_sub(c,buf)
		d = d-1
	assert li_to_int(c) == li_to_int(a) % li_to_int(n)
	return

def test_li_mod_old(A,B):
	Ali = int_to_li(A)
	Bli = int_to_li(B)
	Cli = np.uint32([0xffffffff] * len(Ali))
	Bufli = np.uint32([0xffffffff] * len(Ali))
	li_mod(Cli,Ali,Bli,Bufli)
	assert li_to_int(Cli) == A%B

# Der Aufrufer muss dafür sorgen, dass len(c) gross genug ist
def li_mul(c,a,b):
	#assert len(c) >= len(a) + len(b)
	#assert len(buf) == len(c)
	li_clear(c)
	for i in range(li_bitlen(b)):
		if li_getbit(b,i):
			li_add_shifted(c,a,i)
	assert li_to_int(c) == li_to_int(a) * li_to_int(b)
	return

def test_li_mul(A, B):
	Ali = int_to_li(A)
	Bli = int_to_li(B)
	Cli = int_to_li(0, len(Ali)+len(Bli))
	li_mul(Cli, Ali, Bli)
	return

# Der Aufrufer muss dafür sorgen, dass len(c) gross genug ist
def li_mul_old(c,a,b,buf):
	#assert len(c) >= len(a) + len(b)
	#assert len(buf) == len(c)
	li_clear(c)
	li_clear(buf)
	for i in range(li_bitlen(b)):
		if li_getbit(b,i):
			li_leftshift(buf,a,i)
			li_add(c,buf)
	assert li_to_int(c) == li_to_int(a) * li_to_int(b)
	return

def test_li_mul_old(A, B):
	Ali = int_to_li(A)
	Bli = int_to_li(B)
	Cli = int_to_li(0, len(Ali)+len(Bli))
	Bufli = int_to_li(0, len(Ali)+len(Bli))
	li_mul(Cli, Ali, Bli, Bufli)
	return

# y = x**d mod N
def modexp (X, D, N):
	Y = X
	for i in np.binary_repr(D)[1:]:
		Y = Y**2 % N
		if i=='1':
			Y = Y*X % N
	return Y

def li_modexp(y, x, d, n, buf):
	X = li_to_int(x) ##
	D = li_to_int(d) ##
	N = li_to_int(n) ##
	assert len(y) == len(n)
	assert len(x) == len(n)
	assert len(d) <= len(n)
	assert len(buf) == 2*len(n)
	li_copy(y,x)
	Y = li_to_int(y)
	l = li_bitlen(d)
	for i in range(l-1):
		bitpos = l-2-i
		assert int(np.binary_repr(D)[1:][i]) == li_getbit(d,bitpos) ##
		li_mul(buf, y, y)
		li_mod(buf, n)
		li_copy(y,buf)
		assert li_to_int(y) == Y**2 % N ##
		Y = li_to_int(y) ##
		if li_getbit(d,bitpos):
			li_mul(buf, y, x) # li_mul(buf, buf[:len(n)], x), falls das fehlerfrei
			li_mod(buf, n)
			li_copy(y,buf)
			assert li_to_int(y) == Y*X % N ##
			Y = li_to_int(y) ##
	assert li_to_int(y) == pow(X,D,N) ##
	return

def test_li_modexp(x, d, n):
	nli = int_to_li(n)
	xli = int_to_li(x)
	dli = int_to_li(d)
	l = max((len(xli), len(dli), len(nli)))
	nli = nli + (l-len(nli))* [np.uint32(0)]
	xli = xli + (l-len(xli))* [np.uint32(0)]
	dli = dli + (l-len(dli))* [np.uint32(0)]
	yli = int_to_li(0, l)
	bufli = int_to_li(0, 2*l)
	li_modexp(yli, xli, dli, nli, bufli)
	assert li_to_int(yli) == pow(x,d,n)

def li_modexp_old(y, x, d, n, ybuf, buf):
	X = li_to_int(x) ##
	D = li_to_int(d) ##
	N = li_to_int(n) ##
	assert len(y) == 2*len(n)
	assert len(x) == len(n)
	assert len(d) <= len(n)
	assert len(ybuf) == 2*len(n)
	assert len(buf) == 2*len(n)
	li_copy(y,x)
	Y = li_to_int(y)
	l = li_bitlen(d)
	for i in range(l-1):
		bitpos = l-2-i
		assert int(np.binary_repr(D)[1:][i]) == li_getbit(d,bitpos) ##
		li_mul_old(ybuf, y[:len(n)], y[:-len(n)], buf) # hier wird y ggf verändert?
		li_mod_old(y, ybuf, n, buf)
		assert li_to_int(y) == Y**2 % N ##
		Y = li_to_int(y) ##
		if li_getbit(d,bitpos):
			li_mul_old(ybuf, y[:len(n)], x, buf)
			li_mod_old(y, ybuf, n, buf)
			assert li_to_int(y) == Y*X % N ##
			Y = li_to_int(y) ##
	assert li_to_int(y) == pow(X,D,N) ##
	return

def test_li_modexp_old(x, d, n):
	nli = int_to_li(n)
	xli = int_to_li(x)
	dli = int_to_li(d)
	l = max((len(xli), len(dli), len(nli)))
	nli = nli + (l-len(nli))* [np.uint32(0)]
	xli = xli + (l-len(xli))* [np.uint32(0)]
	dli = dli + (l-len(dli))* [np.uint32(0)]
	yli = int_to_li(0, 2*l)
	ybufli = int_to_li(0, 2*l)
	bufli = int_to_li(0, 2*l)
	li_modexp(yli, xli, dli, nli, ybufli, bufli)
	assert li_to_int(yli) == pow(x,d,n)


def test_modexp():
	x = np.random.randint(1000_0000)
	d = np.random.randint(1000_0000)
	N = np.random.randint(1000_0000)
	assert modexp(x, d, N) == pow(x,d,N)

test_li_to_int()
test_li_bitlen(13)
test_li_bitlen(0)
test_int_to_li(8742598426356209234579876)
test_int_to_li(0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa)
test_li_getbit(0)
test_li_getbit(1)
test_li_getbit(11)
test_li_getbit(0xffffffff4444444111111111aaaaaaaaaa55555555555533333)
test_li_get_word_shifted()
test_li_geq_shifted(2,1,0)
test_li_geq_shifted(2,1,1)
test_li_geq_shifted(2,2,0)
test_li_geq_shifted(0x876123548715,0xf123847698,4)
test_li_geq_shifted(0x8761235487153214f1111110000000,0xf123847698,4)
test_li_geq_shifted(0x8761235487153214f111111aaaaaaaaa555a,0xf123847698,4)
test_li_leftshift(12870987658881,91)
test_li_leftshift(128709876588812345345,111)
test_li_add(321769876659812654, 1273864587537851)
test_li_add(3217698766598124356423665490779877865, 1273864587537851324532453425)
test_li_add_shifted(321769876659812654, 1273864587537851,0)
test_li_add_shifted(321769876659812654, 1273864587537851,1)
test_li_add_shifted(321769876659812654666666, 1273864587537851,33)
test_li_add(3217698766598124356423665490779877865, 1273864587537851324532453425)
test_li_sub(3217698766598126541111, 5234523127386458)
test_li_sub(0xaaaaaaaa675555555555555555588888, 0x7777777aaaabbbbbbbbbbbbbbc)
test_li_sub(0x78658757aaaaaaaaaaa111111111118765876587,0x7687678567567567454612347777777)
test_li_sub_shifted(3217698766598126541111, 5234523127386458)
test_li_sub_shifted(0xaaaaaaaa675555555555555555588888, 0x7777777aaaabbbbbbbbbbbbbbc)
test_li_sub_shifted(0x78658757aaaaaaaaaaa111111111118765876587,0x7687678567567567454612347777777)
test_li_mod(0x7654764555555555555551111111111111,0x8765698798765568764598769786)
test_li_mod(0,1234)
test_li_mod(1234,1)
test_li_mul(0xaaaaaaaa675555555555555555588888, 0x7777777aaaabbbbbbbbbbbbbbc)
test_li_mul(0x78658757aaaaaaaaaaa111111111118765876587,0x7687678567567567454612347777777)
test_li_mul(0x7654764555555555555551111111111111,0x8765698798765568764598769786)
test_li_modexp(3,2134,12345)
test_li_modexp(122134,2234,99444)
test_modexp()
