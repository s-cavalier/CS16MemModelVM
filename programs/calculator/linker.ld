ENTRY(bootuserprog)

SECTIONS
{
    .text 0x00400000 :
    {
        _text_start = .;
        *(.text*)
        *(.rodata*)
        _etext = .;
    }

/* consider adding small data section */


    .data 0x10000000:
    {
        _data_start = .;

        __init_array_start = .;
          KEEP(*(SORT(.init_array.*)))
          KEEP(*(.init_array))
        __init_array_end = .;

        *(.data*)
        _edata = .;
    }

    . = ALIGN(4);

    .bss :
    {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _end = .;
    }
}
