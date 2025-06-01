/* linker.ld — Minimal fixed-address layout for kernel code (no ENTRY/_start) */

/* 
   We place each section at a fixed VMA. 
   - .text at 0x80000180 (already aligned if you reserve 0x180 for vectors), 
   - .data at 0x80100000, 
   - .bss  at 0x80120000. 
*/

ENTRY(main)

SECTIONS
{
    /*------------------------------------------------------------------------------
     * 1) .text at 0x80000180 
     *    (Assuming you reserved 0x80000000–0x8000017F for exception vectors or similar.)
     *------------------------------------------------------------------------------*/
    .text 0x80000180 :
    {
        _text_start = .;        /* optional symbol: start of .text */
        *(.text*)               /* all “.text” input from your .o files */
        *(.rodata*)             /* all read-only data (string literals, etc.) */
        _etext = .;             /* optional symbol: end of .text */
    }

    /*------------------------------------------------------------------------------
     * 2) .data at 0x80100000 
     *    (Place initialized globals/statics here.)
     *------------------------------------------------------------------------------*/
    .data 0x80100000 :
    {
        _data_start = .;        /* optional symbol: start of .data */
        *(.data*)               /* all initialized data from your .o files */
        _edata = .;             /* optional symbol: end of .data */
    }

    /*------------------------------------------------------------------------------
     * 3) .bss at 0x80120000 
     *    (Place zero-initialized globals/statics here.)
     *------------------------------------------------------------------------------*/
    .bss  0x80120000 :
    {
        _bss_start = .;         /* optional symbol: start of .bss */
        *(.bss*)                /* all zero-initialized data from your .o files */
        *(COMMON)               /* any “COMMON” (uninitialized) symbols */
        _end = .;               /* optional symbol: end of all RAM usage */
    }
}
