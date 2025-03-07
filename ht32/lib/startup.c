
#include "startup.h"
#include "types.h"


extern uint32_t _etext;
extern uint32_t _srelocate;
extern uint32_t _erelocate;
extern uint32_t _szero;
extern uint32_t _ezero;

extern void main(void);

/* Reset_Handler()
 *
 * Copy .data section from Flash to RAM and jumps to the main function.
 * Use pointers _etext and _srelocate provided by the linker script.
 */
void Reset_Handler(void) {
	uint32_t *pSrc, *pDest;

	pSrc = &_etext;
	pDest = &_srelocate;

	if (pSrc != pDest) {
		for (; pDest < &_erelocate;) {
			*pDest++ = *pSrc++;
		}
	}

	for (pDest = &_szero; pDest < &_ezero; pDest++) {
		*pDest = 0;
	}

	main();

	// Infinite loop
	while (1);
}
