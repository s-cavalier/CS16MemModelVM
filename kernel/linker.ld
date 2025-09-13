ENTRY(main)


SECTIONS
{
    .text 0x80000000 :
    {
        _text_start = .;
        *(.text*)
        *(.rodata*)
        _etext = .;
    }

    . = ALIGN(8);

/* consider adding small data section */

    . = ALIGN(4);

    .data :
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
