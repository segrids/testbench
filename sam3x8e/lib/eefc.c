
#include "eefc.h"

/*
 * Functions for Flash and GPNVM programming.
 * 
 * The flash of the SAM3x8e organized as follows:
 * 
 * Device | Flash Size | Number of Planes | Pages per Plane | Page Size 
 * ------ | ---------- | ---------------- | --------------- | --------- 
 * SAM3X8 | 512 kB     |  2               | 1024            | 256 
 * 
 * The flash address range is 0x80000 -0x100000. Flash plane 1 starts at 0xC0000.
 * 
 * Depending on GPNVM.1 ROM or Flash is mapped to 0x00000000
 * 
 * Depending on GPNVM.2, flash_start or flash_half is mapped to 0x80000
 * 
 * If code is executed from Plane0 (0x80000-0xC0000) and a flash command is sent 
 * to the responsible EEFC0, the flash command is executed, but the code execution doesn't continue.
 * 
 * ROM code also provides a flash programming function (NMIC exception handler). It's address can
 * be read from the ROM vector table at offset 0x8.
 * 
 * Flashing with the current code works as follows:
 * 	-* The code is compiled and linked to a "first stage boot loader" \p ram.bin
 * 	-* The code is also compiled and linked to a flash image "flash.bin"
 * 	-* The first stage boot loader is loaded into RAM (see testbench.py) and executed.
 * 	-* The first stage boot loader receives the flash image from testbench.Tester, 
 *            writes it into flash, and executes it.
 * 
 * If the code is ececuted from flash plane 0 (resp. plan1), it can be used to flash plane 1 (resp. plane 0).
 * 
 * If the code is excuted from RAM, or from flash plane 1, it can be used to read the Chip Unique ID.
 * 
 * 
 * Writing a flash page is always done in two steps:
 * 	-* Write a full page (256 Bytes) to a write buffer starting at a flash page start address
 * 	-* Write eefc flash command 'Erase Page and write page'
 * 
 * Flash commands (Byte 1 of EEFC_FCR)
 * 
 * Command                       | Code  | Mnemonic
 * ----------------------------- | ----- | -------- 
 * GetFlashDescriptor            | 0x00  | GETD 
 * WritePage                     | 0x01  | WP 
 * WritePageAndLock              | 0x02  | WPL 
 * ErasePageAndWritePage         | 0x03  | EWP 
 * ErasePageAndWritePageThenLock | 0x04  | EWPL 
 * EraseAll                      | 0x05  | EA 
 * SetLockBit                    | 0x08  | SLB 
 * ClearLockBit                  | 0x09  | CLB 
 * GetLockBit                    | 0x0A  | GLB
 * SetGpnvmBit                   | 0x0B  | SGPB
 * ClearGpnvmBit                 | 0x0C  | CGPB
 * GetGpnvmBit                   | 0x0D  | GGPB
 * StartReadUniqueIdentifier     | 0x0E  | STUI
 * StopReadUniqueIdentifier      | 0x0F  | SPUI
 * GetCalibBit                   | 0x10  | GCALB
 * 
 * **/




uint16_t eefc_start_read_unique_identifier(Eefc* p_eefc){
	int flash_command = (0x5a << 24) | 0x0E;
	p_eefc->EEFC_FCR = flash_command;
	while((p_eefc->EEFC_FSR & 1)) {}
	return (uint16_t)p_eefc->EEFC_FSR; // return status from status register
}

uint16_t eefc_stop_read_unique_identifier(Eefc* p_eefc){
	int flash_command = (0x5a << 24) | 0x0F;
	p_eefc->EEFC_FCR = flash_command;
	while(!(p_eefc->EEFC_FSR & 1)) {}
	return (uint16_t)p_eefc->EEFC_FSR; // return status from status register
}



int eefc_page_start_address(Eefc* p_eefc, uint16_t page){
	if (p_eefc == EEFC0){
		return 0x80000 + (int)page * 256;
	} else {
		return 0xC0000 + (int)page * 256;
	}
}

uint16_t eefc_erase_and_write_page(Eefc* p_eefc, uint16_t page){
	int flash_command = (0x5a << 24) | (page << 8) | 0x03; //use ErasePageAndWritePage 0x03
	p_eefc->EEFC_FCR = flash_command;
	while(!(p_eefc->EEFC_FSR & 1)) {}
	return (uint16_t)p_eefc->EEFC_FSR; // return status from status register
}

uint16_t eefc_set_gpnvm_bit(uint8_t bitnumber){
	Eefc* p_eefc = EEFC0;
	int flash_command = (0x5a << 24) | (bitnumber << 8) | 0x0B; //use 0x0B Set GPNVM Bit 
	p_eefc->EEFC_FCR = flash_command;
	while(!(p_eefc->EEFC_FSR & 1)) {}
	return (uint16_t)p_eefc->EEFC_FSR; // return status from status register
}

uint16_t eefc_clear_gpnvm_bit(uint8_t bitnumber){
	Eefc* p_eefc = EEFC0;
	int flash_command = (0x5a << 24) | (bitnumber << 8) | 0x0C; //use 0x0C Clear GPNVM Bit 
	p_eefc->EEFC_FCR = flash_command;
	while(!(p_eefc->EEFC_FSR & 1)) {}
	return (uint16_t)p_eefc->EEFC_FSR; // return status from status register
}

uint16_t eefc_get_gpnvm(uint8_t * p){
	Eefc* p_eefc = EEFC0;
	int flash_command = (0x5a << 24) | 0x0D; //use 0x0D Get GPNVM Bit 
	p_eefc->EEFC_FCR = flash_command;
	*p = (uint8_t)(p_eefc->EEFC_FRR);
	return (uint16_t)p_eefc->EEFC_FSR; // return status from status register
}

