#ifndef APDU_H_
#define APDU_H_

#include "types.h"

int apdu_receive(void);
void apdu_loop(void (*handle)(void));

/**
APDU structure.
**/
struct apdu_t {
	uint8_t cla;
	uint8_t ins;
	uint16_t lc;
	uint8_t data[260];
	uint16_t le;
};

extern struct apdu_t apdu;

#endif
