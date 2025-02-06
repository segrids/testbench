"""A debug information aware debugger."""

import itertools
import textwrap

from ..debugger import Debugger
from .elf import open_elf


class Environment:
    """Accessor for both global and local variables."""

    def __init__(self, dbg):
        self.dbg = dbg

    def __dir__(self):
        if self.dbg.is_halted():
            return [
                v.name
                for v in self.dbg.get_local_variables() + self.dbg.globals
            ]
        else:
            return [v.name for v in self.dbg.globals]

    def __getattr__(self, attr):
        local_vars = {v.name: v for v in self.dbg.get_local_variables()}
        global_vars = {v.name: v for v in self.dbg.globals}

        var = local_vars.get(attr, None)
        if var is None:
            var = global_vars.get(attr, None)
        if var is None:
            raise AttributeError(attr)

        return var.access(self.dbg)


class ELFDebugger(Debugger):
    """Debugger able to interpret the respective ELF file's debug information."""

    def __init__(self, f, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.elf = open_elf(f)
        self.globals = self.elf.get_globals()
        self.env = Environment(self)

    def get_local_variables(self):
        """Returns a list of all local variables.

        Only works when halted.
        """
        return self.elf.get_variables(self.PC(), self)

    def view_source(self, addr=None, context=3):
        """Print the source code placed at `addr` or PC if None is specified.

        `context` sets how many lines before and after the target line of code
        will be printed.
        """
        if addr is None:
            addr = self.PC()

        path, linum, colnum = self.elf.get_line(addr)
        first_line = max(0, linum - context - 1)
        with open(path) as f:
            # TODO: check if linum near EOF breaks
            lines = itertools.islice(f, first_line, linum + context)
            code = ''.join(lines)
        code = textwrap.dedent(code)

        try:
            from pygments import highlight
            from pygments.lexers import get_lexer_for_filename
            from pygments.formatters import TerminalFormatter
            lexer = get_lexer_for_filename(path, stripnl=False)
            code = highlight(code, lexer, TerminalFormatter())
        except ImportError:
            pass

        lines = code.splitlines()

        for i, line in enumerate(lines):
            if i == linum - first_line - 1:
                lines[i] = '>>>  ' + line
            else:
                lines[i] = '     ' + line

        code = '\n'.join(lines)

        print('{}:{}:{}'.format(path, linum, colnum))
        print('=' * 80)
        print(code)
        print('=' * 80)

    def set_breakpoint_source(self, fname, linum, index=None):
        """Set breakpoint (`index`) at `linum` in `fname`.

        Requires Flash Patch Breakpoints to be enabled first.
        """
        addr = self.elf.addr_from_source(fname, linum)
        return self.set_breakpoint(addr, index=index)

    def step_into(self):
        """Continue execution up to the next line of the source code."""
        path, linum, colnum = self.elf.get_line(self.PC())
        while True:
            self.step()
            new_path, new_linum, new_colnum = self.elf.get_line(self.PC())
            if not (new_path == path and new_linum == linum):
                break

    def step_over(self):
        """Continue execution up to the next line of code in the same or a
        previous function.
        """
        fp = self.FP()
        function = self.elf.get_subprogram(self.PC())
        while True:
            self.step_into()

            new_fp = self.FP()
            if new_fp > fp:
                # FIXME: Breaks for leaf (/inlined?) functions which don't use
                # R11 as a frame pointer. Indentify these exceptions via DWARF?
                break

            if self.elf.get_subprogram(self.PC()) == function:
                if new_fp == fp:
                    break
