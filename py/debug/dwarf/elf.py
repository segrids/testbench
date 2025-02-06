"""Wrapper and extension of pyelftools library to extract DWARF debug information."""

from collections import defaultdict
import os

from elftools.elf.elffile import ELFFile
from elftools.dwarf.dwarf_expr import GenericExprVisitor

from .utils import dereference, in_die_range
from .types import parse_type, Var


class Evaluator(GenericExprVisitor):
    """DWARF Expression visitor and evaluator."""

    def __init__(self, die, *args, cfi=None, dbg=None, **kwargs):
        super().__init__(*args, **kwargs)
        self.cfi = cfi
        self.dbg = dbg
        self.die = die
        self.stack = []

    def _after_visit(self, opcode, opcode_name, args):
        # from IPython.core.debugger import Tracer; Tracer()()
        if opcode_name == 'DW_OP_addr':
            self.stack.append(args[0])
        elif opcode_name == 'DW_OP_fbreg':
            offset, = args
            parent = self.die.get_parent()
            if parent.tag != 'DW_TAG_subprogram':
                raise NotImplementedError(opcode_name)
            expr = parent.attributes['DW_AT_frame_base'].value
            visitor = Evaluator(
                parent, parent.cu.structs, cfi=self.cfi, dbg=self.dbg)
            visitor.process_expr(expr)
            self.stack.append(visitor.stack.pop() + offset)
        elif opcode_name == 'DW_OP_call_frame_cfa':
            low_pc = self.die.attributes['DW_AT_low_pc'].value
            pc = self.dbg.PC()
            cfis = [
                cfi for x, cfi in sorted(self.cfi.items()) if low_pc <= x <= pc
            ]
            if cfis:
                cfa = cfis[-1]
                self.stack.append(self.dbg.read_register(cfa.reg) + cfa.offset)
            else:
                raise LookupError(
                    'no call frame information for {:x}'.format(pc))
        else:
            raise NotImplementedError(opcode_name)


class ELF:
    """Parses DWARF debugging information ."""

    def __init__(self, *streams):
        self.elfs = [ELFFile(s) for s in streams]
        dwarfs = [elf.get_dwarf_info() for elf in self.elfs]
        self.cus = [cu for dwarf in dwarfs for cu in dwarf.iter_CUs()]
        self.cfi = {
            entry['pc']: entry['cfa']
            for dwarf in dwarfs for cfi in dwarf.CFI_entries()
            for entry in cfi.get_decoded().table
        }

    def __exit__(self, exc_type, exc_val, exc_tb):
        for elf in self.elfs:
            elf.stream.close()

    def get_globals(self):
        """Parse the DWARF information for global variables."""
        globs = []
        for cu in self.cus:
            for die in cu.get_top_DIE().iter_children():
                if die.tag == 'DW_TAG_variable' and 'DW_AT_name' in die.attributes and 'DW_AT_location' in die.attributes:
                    name = die.attributes['DW_AT_name'].value.decode()
                    exprloc = die.attributes['DW_AT_location'].value
                    visitor = Evaluator(die, cu.structs)
                    visitor.process_expr(exprloc)
                    loc = visitor.stack.pop()
                    ttype = parse_type(dereference(die, 'DW_AT_type'))
                    globs.append(Var(ttype, loc, name))
        return globs

    def get_variables(self, pc, dbg):
        """Parse the DWARF information for local variables."""
        for cu in self.cus:
            if in_die_range(cu.get_top_DIE(), pc):
                break
        else:
            return []

        for die in cu.get_top_DIE().iter_children():
            if die.tag == 'DW_TAG_subprogram' and in_die_range(die, pc):
                break
        else:
            return []

        subprogram = die

        variables = []
        for die in subprogram.iter_children():
            if die.tag in ['DW_TAG_formal_parameter', 'DW_TAG_variable']:
                form = die.attributes['DW_AT_location'].form
                assert form == 'DW_FORM_exprloc' or form.startswith(
                    'DW_FORM_block')
                expr = die.attributes['DW_AT_location'].value
                name = die.attributes['DW_AT_name'].value.decode()
                visitor = Evaluator(die, die.cu.structs, cfi=self.cfi, dbg=dbg)
                visitor.process_expr(expr)
                loc = visitor.stack.pop()
                ttype = parse_type(dereference(die, 'DW_AT_type'))
                variables.append(Var(ttype, loc, name))

        return variables

    def get_subprogram(self, addr):
        for cu in self.cus:
            die = cu.get_top_DIE()
            if in_die_range(die, addr):
                break
        else:
            raise LookupError(
                'no compilation unit found for pc {:#x}'.format(addr))

        for die in cu.get_top_DIE().iter_children():
            if die.tag == 'DW_TAG_subprogram':
                if in_die_range(die, addr):
                    break
        else:
            raise LookupError(
                'no subprogram found for pc {:#x}'.format(addr))

        return die

    def get_line(self, addr):
        for cu in self.cus:
            die = cu.get_top_DIE()
            if in_die_range(die, addr):
                break
        else:
            raise LookupError(
                'no compilation unit found for pc {:#x}'.format(addr))

        line_program = cu.dwarfinfo.line_program_for_CU(cu)
        files = line_program['file_entry']
        dirs = line_program['include_directory']

        entry = max(
            (entry.state for entry in line_program.get_entries()
             if entry.state is not None and entry.state.address <= addr),
            key=lambda s: s.address,
            default=None)

        if entry is None:
            raise LookupError('no line table entry for {:#x}'.format(addr))

        source_file = files[entry.file - 1]
        dir_idx = source_file.dir_index

        compilation_dir = cu.get_top_DIE(
        ).attributes['DW_AT_comp_dir'].value.decode()
        if dir_idx > 0:
            sub_dir = dirs[dir_idx - 1].decode()
            if os.path.isabs(sub_dir):
                source_dir = sub_dir
            else:
                source_dir = os.path.join(compilation_dir, sub_dir)
        else:
            source_dir = compilation_dir

        fname = source_file.name.decode()

        return os.path.join(source_dir, fname), entry.line, entry.column

    def addr_from_source(self, fname, linum):
        # TODO
        # - test for file included multiple times
        # - test breakpoints at for-loops (unexpected breakpoint placement)
        files = defaultdict(list)
        for cu in self.cus:
            lp = cu.dwarfinfo.line_program_for_CU(cu)
            for file_info in lp['file_entry']:
                name = file_info.name.decode()
                files[name].append(cu)
        cus = files[fname]
        if not cus:
            raise LookupError(
                'no debug information for file {!r}'.format(fname))

        entry = min(
            (entry.state for cu in cus
             for entry in cu.dwarfinfo.line_program_for_CU(cu).get_entries()
             if entry.state is not None if linum <= entry.state.line),
            key=lambda e: e.line,
            default=None)

        if entry is None:
            raise LookupError(
                'no debug information for line {} in file {!r}'.format(
                    linum, fname))
        return entry.address


def open_elf(fs):
    """Read one or more ELF files `fs` and return an ELF object."""
    if isinstance(fs, str):
        return ELF(open(fs, 'rb'))
    else:  # assume iterable
        return ELF(*(open(f, 'rb') for f in fs))
