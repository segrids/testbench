/* sets BFD output format */
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)

/* adds working directory to library search path
   like -L . */
SEARCH_DIR(.)


/* Memory Spaces Definitions */
MEMORY
{
	flash (rx)    : ORIGIN = 0x00000000, LENGTH = 0x00020000 /* Flash, 128K */
	ram (rwx)   : ORIGIN = 0x20000000, LENGTH = 0x00004000 /* sram, 16K */
}

/* SECTIONS specifies how input sections are mapped to output sections  */
SECTIONS
{
    .text :
    {
        /*KEEP(*(.vectors))*/
        KEEP(*(.vectors))
        *(.text)
        *(.rodata)
    } > flash

	. = ALIGN(4);

    .data :
    {
		. = ALIGN(4);
		_srelocate = .;
		*(.data);
		. = ALIGN(4);
		_erelocate = .;
    } > ram  AT > flash

	_etext = LOADADDR(.data);

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
	_szero = .;
        *(.bss)
	. = ALIGN(4);
	_ezero = .;
    } > ram

    . = ALIGN(4);

    /* .stack_dummy section doesn't contains any symbols. It is only
       used for linker to calculate size of stack sections, and assign
       values to stack symbols later */
    .stack_dummy :
    {
        *(.stack*)
    } > ram

    /* Set stack top to end of ram, and stack limit move down by
     * size of stack_dummy section */
    __StackTop = ORIGIN(ram) + LENGTH(ram);
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);
    PROVIDE(_sstack = __StackLimit);
    PROVIDE(_estack = __StackTop);
}

