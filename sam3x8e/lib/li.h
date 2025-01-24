#ifndef LI_H_
#define LI_H_

#include "types.h"

/**
LI (Long Integer) structure.
**/
struct li_t {
	uint32_t y[32];
	uint32_t x[32];
	uint32_t d[32];
	uint32_t n[32];
	uint32_t buf[64];
};

extern struct li_t li;

int uint32_bitlen(uint32_t x);
int li_bitlen(uint32_t *a, int len);
int li_getbit(uint32_t *a, int n);
void li_clear(uint32_t *a, int alen);
void li_copy(uint32_t *c, uint32_t *a, int n);
uint32_t li_get_word_shifted(uint32_t *b, int blen, int i, int shift);
int li_geq_shifted(uint32_t *a, int alen, uint32_t *b, int blen, int shift);
void li_add_shifted(uint32_t *c, int clen, uint32_t *a, int alen, int shift);
void li_sub_shifted(uint32_t *c, int clen, uint32_t *a, int alen, int shift);
void li_mod(uint32_t *a, int alen, uint32_t *n, int nlen);
void li_mul(uint32_t *c, int clen, uint32_t *a, int alen, uint32_t *b, int blen);
void modexp(int wordlen);

#endif
