/* linker.ld — Minimal fixed-address layout for kernel code (no ENTRY/_start) */

/* 
   We place each section at a fixed VMA. 
   - .text at 0x00400024 (already aligned if you reserve 0x180 for vectors), 
   - .data at 0x10008000, 
   - .bss  at ?. 
*/

ENTRY(main)

SECTIONS
{
    /*------------------------------------------------------------------------------
     * 1) .text at 0x00400024 
     *    (Assuming you reserved 0x80000000–0x8000017F for exception vectors or similar.)
     *------------------------------------------------------------------------------*/
    .text 0x00400024 :
    {
        _text_start = .;        /* optional symbol: start of .text */
        *(.text*)               /* all “.text” input from your .o files */
        *(.rodata*)             /* all read-only data (string literals, etc.) */
        _etext = .;             /* optional symbol: end of .text */
    }

    /*------------------------------------------------------------------------------
     * 2) .data at 0x10008000 
     *    (Place initialized globals/statics here.)
     *------------------------------------------------------------------------------*/
    .data 0x10008000 :
    {
        _data_start = .;        /* optional symbol: start of .data */
        *(.data*)               /* all initialized data from your .o files */
        _edata = .;             /* optional symbol: end of .data */
    }

    /*————— BSS (RAM, zeroed) —————*/
    . = ALIGN(_edata, 4);  /* align up to 4 bytes after .data */
    .bss : {
        _bss_start = .;         /* symbol: start of .bss */
        *(.bss*)                 /* all .bss input sections */
        *(COMMON)                /* any common symbols (uninitialized globals) */
        _ebss = .;           /* symbol: end of .bss */
    }
}
