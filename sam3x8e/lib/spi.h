
#ifndef SPI_H_
#define SPI_H_

#include "types.h"

typedef struct {
  volatile uint32_t SPI_CR;
  volatile uint32_t SPI_MR;
  volatile uint32_t SPI_RDR;
  volatile uint32_t SPI_TDR;
  volatile uint32_t SPI_SR;
  volatile uint32_t SPI_IER;
  volatile uint32_t SPI_IDR;
  volatile uint32_t SPI_IMR;
  volatile uint32_t space1[4];
  volatile uint32_t SPI_CSR[4];
  volatile uint32_t space2[41];
  volatile uint32_t SPI_WPMR;
  volatile uint32_t SPI_WPSR;
} Spi;

#define SPI0       ((Spi *)0x40008000)


/* function prototypes */

void spi_enable(Spi* p_spi, int mode);

int spi_receive_uint8(Spi* p_spi, uint8_t* byte);
int spi_send_uint8(Spi* p_spi, uint8_t byte);

void spi_close(Spi* p_spi);
void spi_flush_receiver(Spi* p_spi);

uint32_t spi_slave_send_data(Spi* p_spi, uint8_t *buffer, uint32_t buffer_len);
uint32_t spi_slave_receive_data(Spi* p_spi, uint8_t *buffer, uint32_t buffer_len);

uint32_t spi_master_send_data(Spi* p_spi, uint8_t slave_address, uint8_t *buffer, uint32_t buffer_len);
uint32_t spi_master_receive_data(Spi* p_spi, uint8_t slave_address, uint8_t *buffer, uint32_t buffer_len);

#endif
