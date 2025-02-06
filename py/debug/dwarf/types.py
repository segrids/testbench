from elftools.dwarf.constants import DW_ATE_address, DW_ATE_boolean, DW_ATE_signed, DW_ATE_signed_char, DW_ATE_unsigned, DW_ATE_unsigned_char

from .utils import dereference


class Var:
    def __init__(self, ttype, loc, name=None):
        self.type = ttype
        self.loc = loc
        self.name = name

    def access(self, dbg):
        return self.type.access(dbg, self.loc)


class Type:
    def __init__(self, die, volatile, const, name=None):
        self.die = die
        self.volatile = volatile
        self.const = const
        self.name = name

    def at_loc(self, loc):
        return Var(self, loc)


class BaseType(Type):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.size = self.die.attributes['DW_AT_byte_size'].value
        self.encoding = self.die.attributes['DW_AT_encoding'].value

    def access(self, dbg, addr):
        blob = dbg.read_data(addr, self.size)
        # TODO: check if little or big endian, DW_AT_endianity (check that it is not on DW_TAG_base_type, maybe DW_TAG_formal_parameter or DW_TAG_variable)
        if self.encoding == DW_ATE_address:
            return int.from_bytes(blob, 'little')
        elif self.encoding == DW_ATE_boolean:
            return all(b == b'\x00' for b in blob)
        elif self.encoding == DW_ATE_signed:
            return int.from_bytes(blob, 'little', signed=True)
        elif self.encoding == DW_ATE_signed_char:
            return int.from_bytes(blob, 'little', signed=True)
        elif self.encoding == DW_ATE_unsigned:
            return int.from_bytes(blob, 'little')
        elif self.encoding == DW_ATE_unsigned_char:
            return int.from_bytes(blob, 'little')
        else:
            return blob


class StructType(Type):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.size = self.die.attributes['DW_AT_byte_size'].value
        self.members = [(d.attributes['DW_AT_name'].value.decode(),
                         d.attributes['DW_AT_data_member_location'].value,
                         parse_type(dereference(d, 'DW_AT_type')))
                        for d in self.die.iter_children()]

    def access(self, dbg, addr):
        return StructAccessor(dbg, {
            name: t.at_loc(addr + offset)
            for name, offset, t in self.members
        })


class PointerType(Type):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.size = self.die.attributes['DW_AT_byte_size'].value
        self.points_to = parse_type(dereference(self.die, 'DW_AT_type'))

    def access(self, dbg, addr):
        return PointerAccessor(dbg, self, addr)


class ArrayType(Type):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.content = parse_type(dereference(self.die, 'DW_AT_type'))

    def access(self, dbg, addr):
        return ArrayAccessor(dbg, self, addr)


types = {
    'DW_TAG_base_type': BaseType,
    'DW_TAG_structure_type': StructType,
    'DW_TAG_pointer_type': PointerType,
    'DW_TAG_array_type': ArrayType,
}


def parse_type(die):
    volatile = False
    const = False
    name = None

    while die.tag not in types:
        if die.tag == 'DW_TAG_const_type':
            const = True
            die = dereference(die, 'DW_AT_type')
        elif die.tag == 'DW_TAG_volatile_type':
            volatile = True
            die = dereference(die, 'DW_AT_type')
        elif die.tag == 'DW_TAG_typedef':
            if name is not None:
                name = die.attributes['DW_AT_name'].value.decode()
            die = dereference(die, 'DW_AT_type')

    return types[die.tag](die, volatile, const, name)


class ArrayAccessor:
    def __init__(self, dbg, ttype, val):
        self.type = ttype
        self.dbg = dbg
        self.val = val

    def __getitem__(self, key):
        if not isinstance(key, int):
            raise TypeError('{!r} is not an int'.format(key))
        if key < 0:
            raise KeyError('{} is not a positive int'.format(key))
        offset = key * self.type.content.size
        return self.type.content.at_loc(self.val + offset).access(self.dbg)


class PointerAccessor(int):
    def __new__(cls, dbg, ttype, val):
        return super().__new__(cls, val)

    def __init__(self, dbg, ttype, val):
        self.type = ttype
        self.dbg = dbg
        self.val = val

    def __call__(self, *args):
        loc = int.from_bytes(
            self.dbg.read_data(self.val, self.type.size), 'little')
        return self.type.points_to.at_loc(loc).access(self.dbg)

    def __getitem__(self, key):
        if not isinstance(key, int):
            raise TypeError('{!r} is not an int'.format(key))
        if key < 0:
            raise KeyError('{} is not a positive int'.format(key))
        loc = int.from_bytes(
            self.dbg.read_data(self.val, self.type.size), 'little')
        offset = key * self.type.points_to.size
        return self.type.points_to.at_loc(loc + offset).access(self.dbg)


class StructAccessor:
    def __init__(self, dbg, variables):
        self.dbg = dbg
        self.vars = variables

    def __dir__(self):
        return self.vars.keys()

    def __getattr__(self, attr):
        if attr not in self.vars:
            raise AttributeError(attr)
        var = self.vars[attr]
        return var.access(self.dbg)
