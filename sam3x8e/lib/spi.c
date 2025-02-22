/* MIT License

Copyright (c) 2021 by SEGRIDS GmbH <www.segrids.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. 

Author: Frank Schuhmacher <frank.schuhmacher@segrids.com>
*/
 
#include "spi.h"
#include "pmc.h"
#include "pio.h"

/* SPI
 *
 * Data is always read and written at the same time.
 * The MASTER must send data or dummy data 0xFF in order to generate clocks on SCLK.
 * It is suitable that the slave also sends dummy data 0xFF if only reading.
 * In a send-receive function:
 *  - each data write should be followed by an n-fold data read, where 
 *    typically n=1 but sometimes it might be 0 or 2, if the transfer from the receiver
 *    shift register to the receive data register takes a bit longer some point in time.
 *  - Before the first transfer in a communication it is suitable to start with dummy 
 *    reads from the receiver shift register to empty it without writes to the sender 
 *    shift register. This must take place before the peer is expected to send data.
 */



/* spi_enable()
 *
 * Configure the SPI interface either in master (1) or slave (0) mode.
 *
 * NOTE: If the Arduino Due shall be an SPI slave, connect A10 to GND.
 */
void spi_enable(Spi* p_spi, int mode){
	/* enable SPI0 peripheral clock */
	pmc_enable_peripheral_clock(PMC, 24);
	/* assign A25,A26,A27,A28 to Peripheral A, i.e. SPI0 (redundant, also done in main.c) */
	pio_select_peripheral_A(PIOA, 0xf << 25);
	/* SPI MODE Register (SPI_MR)*/
	int MSTR = mode;
	int LLB = 0;     // no loop back (can be enabled for testing)
	int DLYBCS = 240;
	p_spi->SPI_MR = (DLYBCS << 24) | (LLB << 7) | (MSTR << 0);
	/* SPI Chip Select Register (SPI_CSR) */
	int CPOL = 0;    // SCLK is idle low
	int NCPHA = 1;   // data bit evaluated at first clock transition
	int CSNAAT = 1;  // Chip Select rises systematically after each transfer performed on the same slave
	int BITS = 0;    // 8-bit transfer
	int SCBR = 240;  // Serial Clock Baud Rate = 240/MCK	
	int DLYBS = 240; //240; // Delay Before SPCK = 240/MCK
	int DLYBCT = 240; // Delay between consecutive transfers = 32*32/MCK
	p_spi->SPI_CSR[0] = (DLYBCT << 24) | (DLYBS << 16) | (SCBR << 8) | (BITS << 4) | (CSNAAT << 2) | (NCPHA << 1) | (CPOL << 0);
	/* SPI Control Register (SPI_CR) */
	int SPIEN = 1;
	p_spi->SPI_CR = SPIEN << 0; // SPI enable
}


/* spi_flush_receiver() */
void spi_flush_receiver(Spi* p_spi){
	while ((p_spi->SPI_SR & (1 << 0)) != 0){
		p_spi->SPI_RDR;      // <<<<<<<<<<<<<<<<<<<<< read RDR
	}
	/* At function exit, status must be clean again, i.e.
 	 * RDRF == 0: No data has been received since the last read of SPI_RDR. 
 	 * TDRE == 1: last data written in SPI_TDR has been transferred to the serializer
 	 * MODF == 0
 	 * OVRES == 0
 	 * NSSR == 0
 	 * TXEMPTY == 1: SPI_TDR and internal shifter are empty AND delay is over
 	 * UNDES == 0
 	 * SPIENS == 1 */
	uint32_t SPI_SR = p_spi-> SPI_SR;
	if (SPI_SR != ((1 << 16) | (1 << 9) | (1 << 1))){
		/* load SPI_MR and SPI_CSR[0] */
		uint32_t SPI_MR = p_spi->SPI_MR;
		uint32_t SPI_CSR0 = p_spi->SPI_CSR[0];
		/* reset SPI interface */
		p_spi->SPI_CR = 1 << 7;
		/* restore SPI_MR and SPI_CSR[0] */
		p_spi->SPI_MR = SPI_MR;
		p_spi->SPI_CSR[0] = SPI_CSR0;
		/* enable SPI (check if this is necessary!) */
		p_spi->SPI_CR = 1 << 0; // SPI enable		
	}
}

/* spi_close()
 *
 * No need to close an interface so far.
 */
void spi_close(Spi* p_spi){
	return;
}


/* spi_receive_uint8()
 *
 * Receive method for SPI as slave interface.
 */
void spi_receive_uint8(Spi* p_spi, uint8_t* byte){
	p_spi->SPI_TDR = 0x55; /* dummy character to be sent by receiver*/
	/* wait until TDRE == 1: data written in SPI_TDR has been transferred to the serializer */
	while ((p_spi->SPI_SR & (1<<1)) == 0){}
	/* wait until received byte transferred from serializer to SPI_RDR */
	while ((p_spi->SPI_SR & (1<<0)) == 0){}
	*byte = p_spi->SPI_RDR;   // <<<<<<<<<<<<<<<<<<<<<<<<<<<< read RDR
}

/* spi_send_uint8()
 *
 * Send method for SPI as slave interface.
 */
void spi_send_uint8(Spi* p_spi, uint8_t byte){
	p_spi->SPI_TDR = byte;
	/* wait until TDRE == 1: data written in SPI_TDR has been transferred to the serializer */
	while ((p_spi->SPI_SR & (1<<1)) == 0){}
	/* wait until received byte transferred from serializer to SPI_RDR */
	while ((p_spi->SPI_SR & (1<<0)) == 0){}
	p_spi->SPI_RDR;   //dummy read RDR
}

/* spi_master_receive_data()
 *
 * Receive method for SPI as master interface.
 */
int spi_master_receive_data(Spi* p_spi, uint8_t slave_address, uint8_t *buffer, uint32_t buffer_len){
	spi_flush_receiver(p_spi);
	/* At function entry, status must be clean, i.e.
 	 * RDRF == 0: No data has been received since the last read of SPI_RDR. 
 	 * TDRE == 1: last data written in SPI_TDR has been transferred to the serializer
 	 * MODF == 0
 	 * OVRES == 0
 	 * NSSR == 0
 	 * TXEMPTY == 1: SPI_TDR and internal shifter are empty AND delay is over
 	 * UNDES == 0
 	 * SPIENS == 1 */
	uint32_t SPI_SR = p_spi-> SPI_SR;
	if (SPI_SR != ((1 << 16) | (1 << 9) | (1 << 1))){
		return (0xD0 << 8) | SPI_SR;
	}
	for (int i=0; i<buffer_len; i++){
		p_spi->SPI_TDR = 0x55; /* dummy character to be sent by receiver*/
		/* wait until TDRE == 1: data written in SPI_TDR has been transferred to the serializer */
		while ((p_spi->SPI_SR & (1<<1)) == 0){}
		/* wait until received byte transferred from serializer to SPI_RDR */
		while ((p_spi->SPI_SR & (1<<0)) == 0){}
		buffer[i] = p_spi->SPI_RDR;   // <<<<<<<<<<<<<<<<<<<<<<<<<<<< read RDR
	}
	/* wait until TXEMPTY */
	while ((p_spi->SPI_SR & (1<<9)) == 0){}
	/* At function exit, status must be clean again, i.e.
 	 * RDRF == 0: No data has been received since the last read of SPI_RDR. 
 	 * TDRE == 1: last data written in SPI_TDR has been transferred to the serializer
 	 * MODF == 0
 	 * OVRES == 0
 	 * NSSR == 0
 	 * TXEMPTY == 1: SPI_TDR and internal shifter are empty AND delay is over
 	 * UNDES == 0
 	 * SPIENS == 1 */
	SPI_SR = (p_spi-> SPI_SR) & 0xff;
	if (SPI_SR !=  (1 << 1)){
		return (0xF4 << 8) | SPI_SR;
	}
	return 0;
}

/* spi_master_send_data()
 *
 * Send method for SPI as master interface.
 */
int spi_master_send_data(Spi* p_spi, uint8_t slave_address, uint8_t *buffer, uint32_t buffer_len){
	/* At function entry, status must be clean, i.e.
 	 * RDRF == 0: No data has been received since the last read of SPI_RDR. 
 	 * TDRE == 1: last data written in SPI_TDR has been transferred to the serializer
 	 * MODF == 0
 	 * OVRES == 0
 	 * NSSR == 0
 	 * TXEMPTY == 1: SPI_TDR and internal shifter are empty AND delay is over
 	 * UNDES == 0
 	 * SPIENS == 1 */
	uint32_t SPI_SR = p_spi-> SPI_SR;
	if (SPI_SR != ((1 << 16) | (1 << 9) | (1 << 1))){
		return (0xE0 << 8) | SPI_SR;             
	}
	for (int i=0; i<buffer_len; i++){
		p_spi->SPI_TDR = buffer[i];
		/* wait until TDRE == 1: last data written in SPI_TDR has been transferred to the serializer */
		while ((p_spi->SPI_SR & (1<<1)) == 0){}
		/* now it can take a while until RDRF is set by hardware */
		/* wait until data byte received */
		while ((p_spi->SPI_SR & (1<<0)) == 0){}
		/* receiver is expected to send nothing (i.e. 0x55)  as dummy character */
		int SPI_RDR = (p_spi->SPI_RDR) & 0xFF; // <<<<<<<<<<<<<<<<<<<<<< read RDR
	// experimental, used for communication with LPC
		if (SPI_RDR != 0x55){
			return (0xF2 << 8) | SPI_RDR;
		}
	}
	/* wait until TXEMPTY */
	while ((p_spi->SPI_SR & (1<<9)) == 0){}
	/* At function exit, status must be clean again, i.e.
 	 * RDRF == 0: No data has been received since the last read of SPI_RDR. 
 	 * TDRE == 1: last data written in SPI_TDR has been transferred to the serializer
 	 * MODF == 0
 	 * OVRES == 0
 	 * NSSR == 0
 	 * TXEMPTY == 1: SPI_TDR and internal shifter are empty AND delay is over
 	 * UNDES == 0
 	 * SPIENS == 1 */
	SPI_SR = (p_spi-> SPI_SR) & 0xFF;
	if (SPI_SR !=  (1 << 1)){
		return (0xF3 << 8) | SPI_SR;
	}
	return 0;
}


