#ifndef HDQ_H_
#define HDQ_H_

#include "types.h"
#include "tc.h"
#include "pio.h"

typedef struct {
	TcChannel * HDQ_TC;
	Pio * HDQ_PIO;
	int HDQ_PIN;
	int HDQ_POUT;
} Hdq;

Hdq hdq;
#define HDQ0 &hdq

int hdq_init(Hdq *p_hdq, int number);
int hdq_get_pin(Hdq *p_hdq);
void hdq_set_pin(Hdq *p_hdq);
void hdq_clear_pin(Hdq *p_hdq);
void hdq_sleep(Hdq *p_hdq, int tus);
int hdq_detect_low_pulse(Hdq *p_hdq, int *lowtime);
int hdq_receive_uint8(Hdq *p_hdq, uint8_t * byte);
int hdq_apply_low_pulse(Hdq *p_hdq, int lowtime);
int hdq_send_break(Hdq *p_hdq);
int hdq_send_uint8(Hdq *p_hdq, uint8_t byte);
int hdq_master_send_data(Hdq *p_hdq, uint8_t slave_address, uint8_t *buffer, uint32_t buffer_len);
int hdq_master_receive_data(Hdq *p_hdq, uint8_t slave_address, uint8_t *buffer, uint32_t buffer_len);
void hdq_close(Hdq *p_hdq);

#endif
