%-----------------------------------
## Boot loader
%-----------------------------------

### Files

py/
	sam3x8e/bootloader.py
	sam3x8e/pio_status.py
	uart.py
	hexen.py
	loader.py
	apdu.py

sam3x8e/
	loader.ramimage

### Test description

 - Erase Flash and reset to startup sam3x8e ROM code
 - Read ROM code version
 - Read ROM content and compare with known value
 - Read Flash content and assert that flash was erased
 - Read Peripheral Registers and compare with known values
	- UART configuration
	- PIO status
	- PMC configuration
 - Issue EEFC commands
	- check 
 - Flash Data
 - Read Data from Flash and compare
 - Blake2s over free memory in Flash
 - Load RAM binary loader.ramimage
 - Execute RAM binary
 - Do a simple test with loader.py to see that RAM binary is working

%-----------------------------------
## Loader and programmer
%-----------------------------------

### Files

py/
	sam3x8e/programmer.py
	sam3x8e/bootloader.py
	sam3x8e/pio_status.py
	sam3x8e/pmc_status.py
	uart.py
	hexen.py
	loader.py
	apdu.py

sam3x8e/
	loader.ramimage
	loader.image

### Test description

 - Erase Flash and reset to startup sam3x8e ROM code
 - Load RAM binary loader.ramimage
 - Execute RAM binary
 - Flash image loader.image
 - Launch Flash image
 - Tests for all loader.py APDU commands

%-----------------------------------
## Debugger
%-----------------------------------

### Files

py/
	debug/swd.py <<< test object
	debug/ahb.py <<< test object
	debug/debugger.py <<< test object
	loader.py <<< helper

sam3x8e/
	debugger.image <<< test object
	loader.image <<< fixed helper object

### Wiring

 - Debugger at ttyACM0
 - Target at ttyACM1
 - connect Due Pin 3 with SWCLK
 - connect Due Pin 4 with SWD

### Test description

 - Flash loader.image into target
 - Flash debugger.image into debugger
 - Echo command to check that target is alive
 - Check PIO status with Loader class
 - Init debugger
 - Check if halted
 - Halt
 - Read status
 - Check PIO status with Debugger class and compare
 - Resume
 - Echo test
 - set breakpoints
 - launch command and check that it runs until breakpoint

%-----------------------------------
## Adapter
%-----------------------------------

%-----------------------------------
## Sam3x8e
%-----------------------------------

### Test Description
 - PIO status
 - PMC status
 - SPI status

