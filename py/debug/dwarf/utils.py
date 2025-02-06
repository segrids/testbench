"""DWARF utility functions."""


def get_DIE_by_offset(cu, offset):
    dies = cu.iter_DIEs()
    while True:
        try:
            die = next(dies)
        except StopIteration:
            return None
        else:
            reloff = die.offset - cu.cu_offset
            if reloff == offset:
                return die
            elif reloff > offset:
                return None


def dereference(die, attr):
    return get_DIE_by_offset(die.cu, die.attributes[attr].value)


def in_die_range(die, pc):
    low_pc = die.attributes['DW_AT_low_pc'].value
    if die.attributes['DW_AT_high_pc'].form == 'DW_FORM_addr':
        high_pc = die.attributes['DW_AT_high_pc'].value
    else:  # DW_FORM_data*
        high_pc = low_pc + die.attributes['DW_AT_high_pc'].value

    return low_pc <= pc < high_pc
