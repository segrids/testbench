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

Author: David Kuehnemann
"""

from ..hexen import *
from .ahb import AHB


class HaltError(Exception):
    pass


class NotHaltedError(Exception):
    pass


class OutOfFPBsError(Exception):
    pass


"""
class Debugger()

Provide debug functionality on top of the memory access via AHB.

This includes:
    - Halting and stepping/resuming the CPU.
    - Read- and writing CPU registers.
    - Setting Flash Patch Breakpoints (FPBs).
    - Remapping Code (flash) to System space (RAM) using Flash Patches.

Attributes:
        sw_breakpoints: A dictionary of all software breakpoints with their
            indicies as keys.
"""
class Debugger(AHB):
    _DHCSR = 0xe000edf0
    _DCRSR = 0xe000edf4
    _DCRDR = 0xe000edf8
    _FP_CTRL = 0xe0002000
    _FP_REMAP = 0xe0002004

    NUM_FPB_BREAKPOINTS = 8

    @staticmethod
    def _FP_COMP(n):
        """Utility method to calculate the address of the `n`th flash patch comparator."""
        return 0xe0002008 + 4 * n

    @staticmethod
    def _in_RAM(addr):
        """Utility method to check whether an address is in RAM."""
        return addr >= 0x20000000

    @staticmethod
    def is_FPB(index):
        """Check if a breakpoint is a FPB given its index."""
        return index < 8

    @staticmethod
    def is_SWB(index):
        """Check if a breakpoint is a software breakpoint given its index."""
        return index >= 8

    def __init__(self, *args, cache_FPBs=False, **kwargs):
        """Initiate Debugger and configure target's debug port.

        Args:
            cache_FPBs: Optional boolean to cache set FPBs. Recommmended if
                this is the only device setting or unsetting breakpoints as it
                improves performance.
            *args, **kwargs: Passed to base class `AHB`.
        """
        super().__init__(*args, **kwargs)
        self.write_word(self._DHCSR, 0xa05f0001)
        self.write_word(self._FP_CTRL, 0x3)

        self.sw_breakpoints = {}

        self._fpb_cache = None
        if cache_FPBs:
            self.load_cache()

    def halt(self):
        """Halt the CPU."""
        self.write_word(self._DHCSR, 0xa05f0003)
        status = self.read_word(self._DHCSR)
        if not status & 0x00020000:
            raise HaltError()

    def is_halted(self):
        """Check if CPU is halted."""
        return self.read_word(self._DHCSR) & 0x00020000 != 0

    def read_register(self, reg):
        """Read a CPU register.

        R0 = 0
        R1 = 1
        ...
        R11 = FP = 11
        R12 = IP = 12
        R13 = SP = 13
        R14 = LR = 14
        R15 = PC = 15

        There are custom methods generated named `R0`, `R1`, ..., `FP`, `SP`, `LR`, `PC`
        that wrap this one. It is encouraged to use these instead.

        Args:
            reg: An integer index specifying the CPU register.

        Returns:
            The integer content of the given register.
        """
        for _ in range(3):
            if self.read_word(self._DHCSR) & 0x10000:
                self.write_word(self._DCRSR, reg)
                for _ in range(3):
                    if self.read_word(self._DHCSR) & 0x10000:
                        return self.read_word(self._DCRDR)
                else:
                    break
        raise IOError('Debug core not ready.')

    def write_register(self, reg, val):
        """Write a CPU register.

        R0 = 0
        R1 = 1
        ...
        R11 = FP = 11
        R12 = IP = 12
        R13 = SP = 13
        R14 = LR = 14
        R15 = PC = 15

        There are custom methods generated named `R0`, `R1`, ..., `SP`, `LR`, `PC`
        that wrap this one. It is encouraged to use these instead.

        Args:
            reg: An integer index specifying the CPU register.
            val: An integer value to write to the selected register.
        """
        if not self.is_halted():
            raise NotHaltedError(
                'should not write to register when not halted')
        self.write_word(self._DCRDR, val)
        while not self.read_word(self._DHCSR) & 0x00010000:
            # TODO: timeout
            pass
        self.write_word(self._DCRSR, 0x00010000 | reg)

    def step(self):
        """Execute a single instruction when halted."""
        if not self.is_halted():
            raise NotHaltedError('cannot step when not halted')
        bkpt = self.at_breakpoint()
        if bkpt is None:
            self._step()
        else:
            self._step_bkpt(bkpt)

    def _step(self):
        self.write_word(self._DHCSR, 0xa05f0005)

    def resume(self):
        """Resume execution if currently halted."""
        if not self.is_halted():
            raise NotHaltedError('cannot resume execution if not halted')
        bkpt = self.at_breakpoint()
        if bkpt is not None:
            self._step_bkpt(bkpt)
        self.write_word(self._DHCSR, 0xa05f0001)

    def _free_bp_index(self):
        """Utility method which returns the index of the first unused FPB."""
        bkpts = self.breakpoints()
        for i in range(self.NUM_FPB_BREAKPOINTS):
            if i not in bkpts:
                return i
        else:
            raise OutOfFPBsError(
                'All {} Flash Patch Breakpoints are already in use.'.format(
                    self.NUM_FPB_BREAKPOINTS))

    def set_breakpoint(self, addr, index=None):
        """Set a Flash Patch or software breakpoint.

        Args:
            addr: The integer address at which to set the breakpoint.
            index: Optional integer index to select a specific breakpoint. The
                first unused breakpoint is chosen if none is specified.

        Returns:
            The integer index of the set breakpoint.
        """
        if self._in_RAM(addr):  # software breakpoint
            if index is None:
                index = 8
                while index in self.sw_breakpoints:
                    index += 1
            elif index < 8:
                raise ValueError(
                    'All indices below 8 are reserved for Flash Patch Breakpoints.'
                )
            # FIXME: possibly overwriting an already set breakpoint without unsetting it.

            halfword = self.read_halfword(addr)
            self.write_halfword(addr, 0xbe00)
            self.sw_breakpoints[index] = (addr, halfword)
            return index
        else:  # Flash Patch Breakpoint
            if index is None:
                index = self._free_bp_index()

            if addr % 2 != 0:
                raise ValueError(
                    'address {} is not halfword aligned'.format(addr))

            if addr % 4 == 0:
                self.write_word(self._FP_COMP(index), 1 << 30 | addr | 1)
            else:
                self.write_word(
                    self._FP_COMP(index), 2 << 30 | (addr & 0xfffffffc) | 1)

            if self._fpb_cache is not None:
                self._fpb_cache[index] = (addr, 'halfword')

            return index

    def unset_breakpoint(self, bkpt=None, addr=None):
        """Disable a set breakpoint.

        Args:
            bkpt: Optional integer index specifying the breakpoint. Only
                required if no address is provided.
            addr: Optional integer address of the breakpoint. Only required if
                no index is provided.
        """
        if bkpt is None:
            if addr is None:
                raise TypeError(
                    'Address cannot be None if no breakpoint is specified.')
            else:
                bkpts = self.breakpoints()
                try:
                    bkpt = next(idx for idx, (loc, kind) in self.breakpoints()
                                if loc == addr)
                except StopIteration:
                    raise ValueError(
                        'No breakpoint at address {:#x}.'.format(addr))

        if self.is_FPB(bkpt):
            self.write_word(self._FP_COMP(bkpt), 0)
            if self._fpb_cache is not None:
                del self._fpb_cache[bkpt]
        else:
            addr, halfword = self.sw_breakpoints[bkpt]
            self.write_halfword(addr, halfword)
            del self.sw_breakpoints[bkpt]

    def breakpoints(self):
        """Return a joined dictionary of all Flash Patch and software breakpoints."""
        return {**self.fp_breakpoints(), **self.sw_breakpoints}

    def fp_breakpoints(self):
        """Return all active Flash Patch Breakpoints and Remappings."""
        if self._fpb_cache is not None:
            return self._fpb_cache
        else:
            return self._read_breakpoints()

    def load_cache(self):
        """Sync cache with Flash Patch comparators.

        Should usually not have to be invoked manually.
        """
        self._fpb_cache = self._read_breakpoints()

    def _read_breakpoints(self):
        """Utility method to read and parse all Flash Patch comparators.

        Returns:
            A dictionary of all breakpoints with their indicies as keys.
        """
        bkpts = dict()
        for i in range(8):
            val = self.read_word(self._FP_COMP(i))
            if val & 0x01:
                replace = val >> 30
                addr = val & 0x1ffffffc
                if replace == 0:
                    bkpts[i] = (addr, 'remap')
                elif replace == 1:
                    bkpts[i] = (addr, 'halfword')
                elif replace == 2:
                    bkpts[i] = (addr | 2, 'halfword')
                elif replace == 3:
                    bkpts[i] = (addr, 'word')
        return bkpts

    def at_breakpoint(self, addr=None):
        """Check if an address is at a breakpoint.

        Args:
            addr: Optional integer specifying the address. The current program
                counter is used if not provided.

        Returns:
            The breakpoint's index or None otherwise.
        """
        if addr is None:
            addr = self.PC()
        for bp, (bkpt_addr, kind) in self.breakpoints().items():
            if self.is_SWB(bp):
                if bkpt_addr == addr:
                    return bp
            else:
                if kind == 'halfword' and bkpt_addr == addr:
                    return bp
                elif kind == 'word' and bkpt_addr == (addr & 0xfffffffc):
                    return bp
        else:
            return None

    def _step_bkpt(self, bkpt):
        """Private method to step when at a breakpoint.

        Args:
            bkpt: An integer index specifying the breakpoint.
        """
        if self.is_FPB(bkpt):
            self._step_fp(bkpt)
        else:
            self._step_sw(bkpt)

    def _step_fp(self, n):
        """Private method to step when at a FPB.

        Args:
            n: The integer index specifying the breakpoint.
        """
        comp = self.read_word(self._FP_COMP(n))
        self.write_word(self._FP_COMP(n), comp & 0xfffffffe)
        self._step()
        self.write_word(self._FP_COMP(n), comp)

    def _step_sw(self, index):
        """Private method to step when at a software breakpoint.

        Args:
            n: The integer index specifying the breakpoint.
        """
        addr, halfword = self.sw_breakpoints[index]
        self.write_halfword(addr, halfword)
        self._step()
        self.write_halfword(addr, 0xbe00)

    def set_remap_table_loc(self, addr):
        """Set the location of the Flash Patch remap table. Must be in RAM."""
        self.write_word(self._FP_REMAP, addr)
        self.remap_table_loc = addr

    def remap(self, n, addr, val):
        """Remap memory address from code space to system space.

        Args:
            n: The integer index of the Flash Patch comparator. Comparators
                0 through 5 are for instruction loading, 6 & 7 for data loads.
            addr: The integer address in code space to remap.
            val: The integer address in system space to map to.
        """
        self.write_word(self.remap_table_loc + 4 * n, val)
        self.write_word(self._FP_COMP(n), addr | 0x01)

        if self._fpb_cache is not None:
            self._fpb_cache[n] = (addr, 'remap')



    def flash(self,
              plane=0,
              start_page=0,
              path='sam3x8e/flash.bin',
              go=True,
              setGPNVM=True):
        """Flash the target.

        Args:
            plane: Optional integer choosing between plane 0 and plane 1.
            start_page: Optional integer index of the first page to flash.
            path: Optional path to binary image.
            go: Optional boolean to execute the loaded image.
            setGPNVM: Optinal boolean to start from flash after a reset.
        """
        with open(path, 'rb') as fd:
            data = fd.read()
        if len(data) % 256:
            data = data + b'\x00' * (256 - len(data) % 256)

        plane_addr = 0x80000 + 0x40000 * plane

        def issue_command(cmd, arg=0):
            while not (self.read_word(0x400e0a08 + 0x200 * plane) & 1):
                pass
            self.write_word(0x400e0a04 + 0x200 * plane,
                            (0x5a << 24) | (arg << 8) | cmd)

        for i in range(len(data) // 256):
            page = start_page + i
            page_addr = plane_addr + page * 0x100
            d = data[256 * i:256 * (i + 1)]
            self.write_data(page_addr, d)
            issue_command(0x03, page)

        if setGPNVM:
            issue_command(0x0b, 1)
            if plane == 1:
                issue_command(0x0b, 2)
            else:
                issue_command(0x0c, 2)

        if go:
            self.PC(ub32(data[4:8]))
            self.resume()


    def status(d):
        """Print the content of system state registers:"""
        print('--------Core Registers:-----------------')
        print('SP = %08x' % d.SP())
        print('LR = %08x' % d.LR())
        print('PC = %08x' % d.PC())
        print('PSR = %08x' % d.PSR())
        print('PRIMASK = %08x' % d.PRIMASK())
        print('FAULTMASK = %08x' % d.FAULTMASK())
        print('BASEPRI = %08x' % d.BASEPRI())
        print('CONTROL = %08x' % d.CONTROL())
        print('---------System Control Block (SCB):----------------')
        print('Interrupt Control and State Register = %08x' %
              d.read_word(0xE000ED04))
        print('Vector Table Offset Register = %08x' % d.read_word(0xE000ED08))
        print('System Control Register = %08x' % d.read_word(0xE000ED10))
        print('Configuration and Control Register = %08x' %
              d.read_word(0xE000ED14))
        print('Configurable Fault Status Register = %08x' %
              d.read_word(0xE000ED28))
        print('Memory Management Fault Address Register = %08x' %
              d.read_word(0xE000ED34))
        print('Bus Fault Address Register = %08x' % d.read_word(0xE000ED38))
        print(
            '---------Nested Vectored Interrupt Control (NVIC):-------------')
        print('Interrupt enable Register 0 = %08x' % d.read_word(0xE000E100))
        print('Interrupt enable Register 1 = %08x' % d.read_word(0xE000E104))
        print('Interrupt pending Register 0 = %08x' % d.read_word(0xE000E200))
        print('Interrupt pending Register 1 = %08x' % d.read_word(0xE000E204))
        print(
            'Interrupt Active Bit Register 0 = %08x' % d.read_word(0xE000E300))
        print(
            'Interrupt Active Bit Register 1 = %08x' % d.read_word(0xE000E304))
        print('--------Power Management Controller (PMC):------------')
        print('System Clock Status Register = %08x' % d.read_word(0x400E0608))
        print('Peripheral Clock Status Register 0 = %08x' %
              d.read_word(0x400E0618))
        print('Status Register = %08x' % d.read_word(0x400E0668))
        print('Peripheral Clock Status Register 1 = %08x' %
              d.read_word(0x400E0708))

    def tc(self, channel):
        module = channel // 3
        baseaddress = 0x40080000 + 0x4000 * (channel // 3) + 0x40 * (
            channel % 3)
        TC_CCR = self.read_word(baseaddress + 0x0)
        print('Channel Control Register = %08x' % TC_CCR)
        TC_CMR = self.read_word(baseaddress + 0x04)
        print('Channel Mode Register = %08x' % TC_CMR)
        print('Counter Value = %08x' % self.read_word(baseaddress + 0x10))
        print('Register A = %08x' % self.read_word(baseaddress + 0x14))
        print('Register B = %08x' % self.read_word(baseaddress + 0x18))
        print('Register C = %08x' % self.read_word(baseaddress + 0x1C))
        TC_SR = self.read_word(baseaddress + 0x20)
        print('Status Register = %08x' % TC_SR)
        TC_IMR = self.read_word(baseaddress + 0x2C)
        print('Interrupt Mask Register = %08x' % TC_IMR)


def _add_reg(name, reg):
    """Utility method to add named register access methods to `Debugger`

    Args:
        name: String name of the register and the access method.
        reg: Integer index of the register."""
    def rw_reg(self, val=None):
        if val is None:
            return self.read_register(reg)
        else:
            self.write_register(reg, val)

    # Debugger.<name> = rw_reg
    setattr(Debugger, name, rw_reg)


for i in range(16):
    _add_reg('R{}'.format(i), i)
_add_reg('FP', 11)
_add_reg('SP', 13)
_add_reg('LR', 14)
_add_reg('PC', 15)
_add_reg('PSR', 16)
_add_reg('PRIMASK', 17)
_add_reg('FAULTMASK', 18)
_add_reg('BASEPRI', 19)
_add_reg('CONTROL', 20)
