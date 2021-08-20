#ifndef EEFC_H_
#define EEFC_H_

#include "types.h"


typedef struct {
  volatile uint32_t EEFC_FMR;
  volatile uint32_t EEFC_FCR;
  volatile uint32_t EEFC_FSR;
  volatile uint32_t EEFC_FRR;
} Eefc;


#define EEFC0       ((Eefc    *)0x400E0A00)
#define EEFC1       ((Eefc    *)0x400E0C00)


uint16_t eefc_start_read_unique_identifier(Eefc* p_eefc);
uint16_t eefc_stop_read_unique_identifier(Eefc* p_eefc);
int eefc_page_start_address(Eefc* p_eefc, uint16_t page);
uint16_t eefc_erase_and_write_page(Eefc* p_eefc, uint16_t page);
uint16_t eefc_set_gpnvm_bit(uint8_t bitnumber);
uint16_t eefc_clear_gpnvm_bit(uint8_t bitnumber);
uint16_t eefc_get_gpnvm(uint8_t * p);


#endif
