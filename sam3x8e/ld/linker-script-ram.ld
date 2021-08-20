/* sets BFD output format */
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)

/* adds working directory to library search path
   like -L . */
SEARCH_DIR(.)

SECTIONS
{
    . = 0x20001000; /* ROM program hangs if 0x20000800-0x20001000 written*/
    .text :
    {
        KEEP(*(.vectors))
        *(.text)
        *(.rodata)
    }

    .data :
    {
        *(.data);
    }

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        *(.bss)
    }

    PROVIDE(_estack = 0x20010000);
}
