""" MIT License

Copyright (c) 2021 by Frank Schuhmacher <frank.schuhmacher@segrids.com>

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

Author: Frank Schuhmacher
"""

"""
spi_status()

Read registers of the sam3x8e to analyse the status of the SPI0 peripheral (ID=24).

arg:
    t = an instance of Samba, Loader, Tester, Programmer, or Debugger
"""
def spi_status(t):
    # check if the following 4 pins are assigned to SPI0:
    # A25      | A | MISO      | Due SPI     | SPI0
    # A26      | A | MOSI      | Due SPI     | SPI0
    # A27      | A | SCLK      | Due SPI     | SPI0
    # A28 (C29)|PIO| nSS       | Due Pin 10  | SPI0
    PIO_PSR0 = t.read_word(0x400E0E08)
    if PIO_PSR0 & (0xf << 25):
        print("ERROR: pins A25-A28 not all assigned to SPI0")

    # check if the SPI0 peripheral (ID=24) is clocked
    PMC_PCSR0 = t.read_word(0x400E0618) # PMC Peripheral Clock Status Register 0
    if PMC_PCSR0 & (1<<24):
        print("SPI0 is clocked")
    else:
        print("SPI0 is NOT clocked!")

    SPI_SR = t.read_word(0x40008010)
    # check if SPI0 peripheral is enabled
    if SPI_SR & (1<<16):
        print("SPI0 is enabled")
    else:
        print("SPI0 is NOT enabled!")
    if SPI_SR & (1<<10):
        print("UNDES=1: A transfer begins whereas no data has been loaded in the Transmit Data Register.")
    if SPI_SR & (1<<9):
        print("TXEMPTY=1: SPI_TDR and internal shifter are empty. If a transfer delay has been defined, TXEMPTY is set after the completion of such delay.")
    else:
        print("TXEMPTY=0: data is written in SPI_TDR")
    if SPI_SR & (1<<8):
        print("NSSR=1: A rising edge occurred on NSS pin since last read.")
    else:
        print("NSSR=0: No rising edge detected on NSS pin since last read.") 
    if SPI_SR & (1<<3):
        print("OVRES=1: An overrun has occurred since the last read of SPI_SR.")
    else:
        print("OVRES=0: No overrun has been detected since the last read of SPI_SR.") 
    if SPI_SR & (1<<2):
        print("MODF=1: A Mode Fault occurred since the last read of the SPI_SR.")
    else:
        print("MODF=0: No Mode Fault has been detected since the last read of SPI_SR.") 
    if SPI_SR & (1<<1):
        print("TDRE=1: The last data written in the Transmit Data Register has been transferred to the serializer.")
    else:
        print("TDRE=0: Data has been written to SPI_TDR and not yet transferred to the serializer.") 
    if SPI_SR & (1<<0):
        print("RDRF=1: Data has been received and the received data has been transferred from the serializer to SPI_RDR since the last read of SPI_RDR.")
    else:
        print("RDRF=0: No data has been received since the last read of SPI_RDR") 

    SPI_MR = t.read_word(0x40008004)
    DLYBCS = SPI_MR >> 24
    PCS = (SPI_MR >> 16) & 0xFF
    print("DLYBCS: Delay Between Chip Selects = %dms (if MCK at 24MHz)" %(DLYBCS//24000))
    
