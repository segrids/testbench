/* sets BFD output format */
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)

/* adds working directory to library search path
   like -L . */
SEARCH_DIR(.)

MEMORY
{
	/* -------- 0x20000000:0x20002000 = test program ------------------------------------------------------ */
	module_seg (rwx) : ORIGIN = 0x20000000, LENGTH =  0x2000  /* module binay including ivt max. 1680 bytes */
	/* matrix_seg (rwx)    : ORIGIN = 0x20000700, LENGTH = 0x1900   320 Test Items               1600 words */
	/* -------- 0x20002000:0x20002010 = test debug output ------------------------------------------------- */
	inbuf_seg (rwx)  : ORIGIN = 0x20002000, LENGTH =   0x10   /* to-be-initialized buffer           4 words */
	/* -------- 0x20002010:0x20003a00 = test debug output ------------------------------------------------- */
	rythm_seg (rwx)  : ORIGIN = 0x20002010, LENGTH =  0xff0   /* 1024 rythm triples (nn,pp,tt)   1024 words */
	stres_seg (rwx)  : ORIGIN = 0x20003000, LENGTH =  0xa00   /* 320 stimuli and responses        640 words */
	/* -------- 0x20003a00:0x20003c00 = module stack ------------------------------------------------------ */
	stack_seg (rwx)  : ORIGIN = 0x20003a00, LENGTH =  0x200   /* module stack                     128 words */
	/* -------- 0x20003c00:0x20004000 = reserved for flash code ------------------------------------------- */
	forbid_seg (rwx) : ORIGIN = 0x20003c00, LENGTH =  0x400   /* flash .bss segment and stack     256 words */
}

SECTIONS
{
	.text :
	{
		KEEP(*(.vectors))
		*(.text);
	} > module_seg


	PROVIDE(_smodule = ORIGIN(module_seg));
	PROVIDE(_emodule = ORIGIN(module_seg) + LENGTH(module_seg));
	PROVIDE(_sinbuf = ORIGIN(inbuf_seg));
	/*PROVIDE(_smatrix = ORIGIN(matrix_seg));*/
	PROVIDE(_srythm = ORIGIN(rythm_seg));
	PROVIDE(_sstres = ORIGIN(stres_seg));
	PROVIDE(_smstack = ORIGIN(stack_seg));
	PROVIDE(_emstack = ORIGIN(stack_seg) + LENGTH(stack_seg));
}





