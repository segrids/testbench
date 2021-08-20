#ifndef SWD_H_
#define SWD_H_

#include "types.h"

extern const uint32_t swd_clock_pin_c;
extern const uint32_t swd_data_pin_c;
extern const uint32_t swd_reset_pin_c;
void swd_line_reset(void);
void swd_target_reset(void);
int swd_read(uint8_t req, uint32_t * value);
int swd_write(uint8_t req, uint32_t value);

#endif
