#include "li.h"


struct li_t li;

int uint32_bitlen(uint32_t x){
    int ret = 0;
    while (x != 0){
        x = x >> 1;
        ret = ret + 1;
	}
    return ret;
}

int li_bitlen(uint32_t *a, int len) {
    for (int i=len-1; i>=0; i--){
        if (a[i] != 0) {
            return (i << 5) + uint32_bitlen(a[i]);
		}
	}
    return 0;
}

int li_getbit(uint32_t *a, int n){
	return (int)(a[n>>5] >> (n & 0x1f)) & 1;
}

void li_clear(uint32_t *a, int alen){
	for (int i=0; i<alen; i++){
		a[i] = 0;
	}
}

void li_copy(uint32_t *c, uint32_t *a, int n){
	for (int i=0; i<n; i++){
		c[i] = a[i];
	}
}

uint32_t li_get_word_shifted(uint32_t *b, int blen, int i, int shift){
	// i=1, shift=0
	int wordshift = shift>>5;
	int bitshift = shift & 0x1f;
	if (bitshift == 0){ // Fallunterscheidung, weil RV32 Rechtsshift um 32 nicht kann
		if (i < wordshift){
    	    return 0;
		} else if (i == wordshift){
    	    return b[0];
		} else if (i < blen + wordshift){
    	    return b[i-wordshift];
    	} else if (i >= blen + wordshift){
    	    return 0;
		}
    } else {
		if (i < wordshift){
    	    return 0;
		} else if (i == wordshift){
    	    return b[0] << bitshift;
		} else if (i < blen + wordshift){
    	    return (b[i-wordshift] << bitshift) + (b[i-wordshift-1] >> (32-bitshift));
    	} else if (i == blen + wordshift){
    	    return b[blen-1] >> (32-bitshift);
    	} else if (i > blen + wordshift){
    	    return 0;
		}
	}
	return 0; // to avoid compiler warning
}

int li_geq_shifted(uint32_t *a, int alen, uint32_t *b, int blen, int shift){
    for (int i=alen-1; i>=0; i--){ 
        uint32_t shifted_bi = li_get_word_shifted(b,blen,i,shift);
        if (a[i] < shifted_bi){
            return 0; // False
        } else if (a[i] > shifted_bi) {
            return 1; // True
		}
	}
    return 1; // True
}

void li_add_shifted(uint32_t *c, int clen, uint32_t *a, int alen, int shift){
    uint32_t carry = 0;
    for (int i=0; i<clen; i++){
		c[i] = c[i] + carry;
		uint32_t carry1 = (uint32_t)(c[i] < carry);
		uint32_t adder = li_get_word_shifted(a,alen,i,shift);
		c[i] = c[i] + adder; 
    	uint32_t carry2 = (uint32_t)(c[i] < adder);
    	carry = carry1 + carry2;
	}
    return;
}

void li_sub_shifted(uint32_t *c, int clen, uint32_t *a, int alen, int shift){
    uint32_t carry = 1;
    for (int i=0; i<clen; i++){
		c[i] = c[i] + carry;
		uint32_t carry1 = (uint32_t)(c[i] < carry);
		uint32_t adder = li_get_word_shifted(a,alen,i,shift);
		adder = ~adder;
		c[i] = c[i] + adder; 
    	uint32_t carry2 = (uint32_t)(c[i] < adder);
    	carry = carry1 + carry2;
	}
    return;
}

void li_mod(uint32_t *a, int alen, uint32_t *n, int nlen){
    int d = li_bitlen(a, alen) - li_bitlen(n, nlen);
    while (d >= 0) {
        if (li_geq_shifted(a, alen, n, nlen, d)){
            li_sub_shifted(a, alen, n, nlen, d);
		}
        d = d-1;
	}
    return;
}

void li_mul(uint32_t *c, int clen, uint32_t *a, int alen, uint32_t *b, int blen){
    li_clear(c, clen);
    for (int i=0; i<li_bitlen(b, blen); i++){
        if (li_getbit(b,i)){
            li_add_shifted(c,clen,a,alen,i);
		}
	}
    return;
}

void li_modexp(int wordlen){
	// Y = X
	li_clear(li.buf, 64);
	li_clear(li.y, 32);
    li_copy(li.y, li.x, wordlen);
	int l = li_bitlen(li.d, wordlen);
	for (int i=0; i<l-1; i++){
		int bitpos = l-2-i;
        // Y = Y**2 % N
		li_mul(li.buf, 2*wordlen, li.y, wordlen, li.y, wordlen);
		li_mod(li.buf, 2*wordlen, li.n, wordlen);
    	li_copy(li.y, li.buf, wordlen); 
        if (li_getbit(li.d, bitpos)){
            // Y = Y*X % N
			li_mul(li.buf, 2*wordlen, li.y, wordlen, li.x, wordlen);
			li_mod(li.buf, 2*wordlen, li.n, wordlen);
    		li_copy(li.y, li.buf, wordlen); 
		}
	}
    return;
}
