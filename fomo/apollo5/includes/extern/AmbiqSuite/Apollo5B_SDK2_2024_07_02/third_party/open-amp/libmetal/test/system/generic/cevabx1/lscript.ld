OUTPUT(a.elf)

ENTRY(__cxd_inttbl_start)

__internal_code_size = DEFINED(__internal_code_size)  ? __internal_code_size  : 256k;
__internal_data_size = DEFINED(__internal_data_size ) ? __internal_data_size  : 512k;
__external_mem_size  = DEFINED(__external_mem_size  ) ? __external_mem_size   : 1M;

__malloc_size        = DEFINED(__malloc_size) ? __malloc_size : 512k;
__stack_size         = DEFINED(__stack_size) ? __stack_size : 32k;
__arg_sect_size      = DEFINED(__arg_sect_size) ? __arg_sect_size : 400;

MEMORY
{
    INTERNAL_CODE       (rx)  : ORIGIN = 0x00000000, LENGTH = __internal_code_size
    INTERNAL_DATA       (rw)  : ORIGIN = 0x00000000, LENGTH = __internal_data_size
    /* Use FPGA BRAM: axi_bram_ctrl_0 - 0xA000_0000*/
    EXTERNAL_DRAM       (rwx) : ORIGIN = 0xA0000000, LENGTH = __external_mem_size

}

SECTIONS {

    .code.internal : ALIGN(0x20) {
        /** The interrupt vector resides at address zero and contains the nmi
         * and maskable interrupt handlers */
        . = 0x0;
        KEEP(*(.inttbl))
        . = ALIGN(0x20);
        KEEP(*(.sinttbl))
        /* The __call_saved* functions need to be placed at low addresses for
         * calling with absolute call instructions */
        *(.text.__call_saved*)
        *(.text.__cxd_call_saved*)

        . = ALIGN(0x100);
        *(code_isr code_isr.*)
    } >INTERNAL_CODE

    .text : ALIGN(0x20) {
        PROVIDE(__text_start = ABSOLUTE(.));
        *(.text .text.*)
        PROVIDE(__text_end = ABSOLUTE(.));
    } >INTERNAL_CODE

    .data.internal : ALIGN(0x20) {
        PROVIDE(__data_internal_start = ABSOLUTE(.));
        /* Disable the cloning by placing the end marker right at the start */ 
        PROVIDE(__data_internal_end = ABSOLUTE(.));
        PROVIDE(__data_internal_size = ABSOLUTE(__data_internal_end - __data_internal_start));

        /* Don't map any data at address zero to avoid issues with C NULL
         * pointer checks */
        . += 0x8;

        PROVIDE(__data_start = ABSOLUTE(.));
        *(.data .data.*)
        PROVIDE(__data_end = ABSOLUTE(.));
        PROVIDE(__data_size = +__data_end - __data_start);

        PROVIDE(__sdata_start = ABSOLUTE(.));
        *(.sdata .sdata.*)
        PROVIDE(__sdata_end = ABSOLUTE(.));
        PROVIDE(__sdata_size = +__sdata_end - __sdata_start);

   } >INTERNAL_DATA

     .data.internal.clone (NOLOAD) : ALIGN(0x20) {
        PROVIDE(__data_internal_clone_start = ABSOLUTE(.));
		. = ABSOLUTE(. + __data_internal_size);
   } >EXTERNAL_DRAM

   data_internal_loadable_addr = __data_internal_clone_start;

   .data.internal.ro : ALIGN(0x20) {
        PROVIDE(__data_internal_ro_start = ABSOLUTE(.));
        PROVIDE(__rodata_start = ABSOLUTE(.));
        *(.rodata .rodata.*)
        PROVIDE(__rodata_end = ABSOLUTE(.));
        PROVIDE(__rodata_size = +__rodata_end - __rodata_start);

        /* program arguments are loaded by `_start` from `__arg_sect_start`.
         * When the user has set a zero size for the section, argc, and argv
         * will be zero and NULL, respectively. Can be placed in external memory 
         * (read-only).
         */
        PROVIDE(__arg_sect_start = .);
        . = . + (__arg_sect_size ? __arg_sect_size + 4 : 0);
        PROVIDE(__arg_sect_end = .);

        PROVIDE(__data_internal_ro_end = ABSOLUTE(.));
        PROVIDE(__data_internal_ro_size = __data_internal_ro_end - __data_internal_ro_start);
    } >INTERNAL_DATA

	
      .cst.call : ALIGN(4)
      {
      	__cst_call_start = ABSOLUTE(.);
      	*(.cst.call)
      	__cst_call_end = ABSOLUTE(.);
      } >INTERNAL_DATA

      .cst.mov : ALIGN(4)
      {
      	__cst_mov_start = ABSOLUTE(.);
      	*(.cst.mov)
      	__cst_mov_end = ABSOLUTE(.);
      } >INTERNAL_DATA
      
    

    .bss (NOLOAD) : ALIGN(0x20) {
        PROVIDE(__bss_start = ABSOLUTE(.));
        *(.bss .bss.*)
        PROVIDE(__common_start = ABSOLUTE(.));
        *(COMMON)
        PROVIDE(__common_end = ABSOLUTE(.));
        PROVIDE(__common_size = +__common_end - __common_start);

        PROVIDE(__bss_end = ABSOLUTE(.));
        PROVIDE(__bss_size = +__bss_end - __bss_start);
    } >INTERNAL_DATA

    .bss1 (NOLOAD) : ALIGN(0x20) {
        /**
         * `.bss1` is for large zero-initialized symbols that do not fit in
         * internal data
         */
        PROVIDE(__bss1_start = ABSOLUTE(.));
        *(.bss1 .bss1.*)
        PROVIDE(__large_common_start = ABSOLUTE(.));
        *(LARGE_COMMON)
        PROVIDE(__large_common_end = ABSOLUTE(.));
        PROVIDE(__large_common_size = +__large_common_end - __large_common_start);
        PROVIDE(__bss1_end = ABSOLUTE(.));
        PROVIDE(__bss1_size = +__bss1_end - __bss1_start);
    } >EXTERNAL_DRAM

	.data.external : ALIGN(0x20) {
        /** .data1, .rodata1, .sdata1 are all for large symbols which cannot
         * fit in limited internal memory. We put them in external memory by
         * default. */
        PROVIDE(__data_external_start = ABSOLUTE(.));
        /* Disable the cloning by placing the end marker right at the start */ 
		PROVIDE(__data_external_end = ABSOLUTE(.));
        PROVIDE(__data_external_size = ABSOLUTE(__data_external_end - __data_external_start));

        PROVIDE(__data1_start = ABSOLUTE(.));
        *(.data1 .data1.*)
        PROVIDE(__data1_end = ABSOLUTE(.));

        PROVIDE(__sdata1_start = ABSOLUTE(.));
        *(.sdata1 .sdata1.*)
        PROVIDE(__sdata1_end = ABSOLUTE(.));
        PROVIDE(__sdata1_size = +__sdata1_end - __sdata1_start);

    } >EXTERNAL_DRAM

	.data.external.clone (NOLOAD) : ALIGN(0x20) {
        PROVIDE(__data_external_clone_start = ABSOLUTE(.));
		. = ABSOLUTE(. + __data_external_size);
   } >EXTERNAL_DRAM

	data_external_loadable_addr = __data_external_clone_start;

    .data.external.ro : ALIGN(0x20) {
        /** .data1, .rodata1, .sdata1 are all for large symbols which cannot
         * fit in limited internal memory. We put them in external memory by
         * default. */
        PROVIDE(__data_external_ro_start = ABSOLUTE(.));

        PROVIDE(__rodata1_start = ABSOLUTE(.));
        *(.rodata1 .rodata1.*)

        PROVIDE(__rodata1_end = ABSOLUTE(.));
        PROVIDE(__rodata1_size = +__rodata1_end - __rodata1_start);

        /* Constructors and destructors are called once per program invocation,
         * so are never in the hot path; they shouldn't waste space in limited
         * internal memory so we place them in slower, external memory */

        . = ALIGN(4); /* constructors must be aligned on a word boundary */
        PROVIDE(__init_array_start = ABSOLUTE(.));
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)));
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array*) SORT_BY_INIT_PRIORITY(.ctors*)));
        PROVIDE(__init_array_end = ABSOLUTE(.));

        PROVIDE(__fini_array_start = ABSOLUTE(.));
        /* destructors are run in reverse order of their priority */
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)));
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array*) SORT_BY_INIT_PRIORITY(.dtors*)));
        PROVIDE(__fini_array_end = ABSOLUTE(.));

        PROVIDE(__data_external_ro_end = ABSOLUTE(.));
        PROVIDE(__data_external_ro_size = __data_external_ro_end - __data_external_ro_start);
    } >EXTERNAL_DRAM

    __STACK_SECT (NOLOAD) : ALIGN(0x10) {
        KEEP(*(__stack_start));
            __stack_start = ABSOLUTE(.);
        . = . + __stack_size;
        KEEP(*(__stack_end));
            __stack_end = ABSOLUTE(.);
    } >INTERNAL_DATA

    __MALLOC_SECT (NOLOAD) : ALIGN(0x10) {
        KEEP(*(__malloc_start));
            __malloc_start = ABSOLUTE(.);
        . = . + __malloc_size;
        KEEP(*(__malloc_end));
            __malloc_end = ABSOLUTE(.);
    } >EXTERNAL_DRAM


    /DISCARD/ : {
        /* Discarding .note.CEVA-arch saves a fair amount of space but
         * confounds the restriction checker. YMMV */
        /* *(.note.CEVA-arch) */
        *(.comment)
        *(.note.GNU-stack)
        /* The X-DSP ABI uses a custom relocation format stored in its own
         * section. These are left in the binary by default but are unneeded. */
        *(.ceva_reloc)
    }
}
