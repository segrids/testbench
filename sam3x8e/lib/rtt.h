#ifndef RTT_H_
#define RTT_H_

#include "types.h"

typedef struct {
  volatile uint32_t RTT_MR;
  volatile uint32_t RTT_AR;
  volatile uint32_t RTT_VR;
  volatile uint32_t RTT_SR;
} Rtt;

#define RTT ((Rtt *)0x400E1A30)


void rtt_init(Rtt* p_rtt);
void rtt_sleep(Rtt* p_rtt, int milliseconds);

#endif
