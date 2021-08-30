# Testbench
Test control with **python3** and the **arduino due**.

## Contents
Two main contents:
- A python3 package with **loader**, **programmer**, **tester**, **debugger** and **adapter classes**. The import of these classes makes the ipython shell our test command center. Each class communicates via application protocol data units (APDUs) with a corresponding handler on the arduino due target. 
- Self-contained C-code for the sam3x8e / arduino due including a **driver library**, **loader**, **programmer**, **tester**, **debugger** and **adapter handlers**, and **Makefiles** for the ARM gnu toolchain.

## Applications
- A **programmer** for flashing code into the arduino due. 
- A **debugger** with python control the arduino due as debugger hardware
  for debugging any system supporting the SWD protocol.
- An **adapter** for protocols SPI, I2C, UART and HDQ with
  the arduino due as adapter hardware.

In combination these applications form a **testbench** to be used for:
  - HW testing,
  - protocol testing, and
  - measurement control.
 
 We use it for example to control measurements in **side channel anaylsis** .
 
 ## Howto
 We are going to publish a few jupyter-notebooks and youtube videos to show how to use the testbench in practical examples.
 
 ## Requirements
 The python3 packages and the toolchain were tested on a Debian 10 system with the following packages installed:
  - gcc-arm-none-eabi (15:7-2018-q2-6)
  - binutils-arm-none-eabi (2.31.1-12+11)
  - ipython3 (5.8.0-1)
  - python3-serial (3.4-4)
  
