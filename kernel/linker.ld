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

    .data 0x80001000 :
    {
        _data_start = .;
        *(.data*)
        _edata = .;
    }

    .bss  0x80001200 :
    {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _end = .;
    }
}
