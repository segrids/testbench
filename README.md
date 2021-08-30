# Testbench
Software for IC programming, testing, debugging and hacking using
**python3**, **gcc** and the **arduino due**.

## Advantages
- Test control from the python shell (ipyhon3 or jupyper-noteboook).
- Debug control from the python shell with the arduino due as debug hardware.
- Arduino Due as adapter for SPI, I2C, UART and HDQ.
- MIT licensed SAM3x8e libraries.
- No need for any IDE.

## Contents
- A python3 package with **samba**, **loader**, **programmer**, **tester**, **debugger** and **adapter classes**. 
The samba class talks to the SAM-BA ROM code of the SAM3x8e MCU. The other classes communicate 
with a corresponding handler on the arduino due target via application protocol data units (APDUs).
- Self-contained C-code for the sam3x8e / arduino due including a **driver library**, **loader**, **programmer**, **tester**, **debugger** and **adapter** handlers, and **Makefiles** for the ARM gnu toolchain.

## Applications
- A **programmer** for flashing code into the arduino due. 
- A **debugger** with python control the arduino due as debugger hardware
  for debugging any system supporting the SWD protocol.
- An **adapter** for protocols SPI, I2C, UART and HDQ with
  the arduino due as adapter hardware.
 
## Howto
 We are going to publish a few jupyter-notebooks and youtube videos to show how to use the testbench in practical examples.
 
## Requirements
 The software was tested on a Debian 10 system with the following packages installed:
  - gcc-arm-none-eabi (15:7-2018-q2-6)
  - binutils-arm-none-eabi (2.31.1-12+11)
  - ipython3 (5.8.0-1)
  - python3-serial (3.4-4)

The application requires at least one arduino due board. The adapter application additionally requires any target with SPI, I2C, or HDQ interface. The debugger application requires any target with SWD interface.
