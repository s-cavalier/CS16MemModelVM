ENTRY(main)
__global_pointer = 0x80007500;

SECTIONS
{
    .text 0x80000000 :
    {
        _text_start = .;
        *(.text*)
        *(.rodata*)
        _etext = .;
    }

    .data 0x80005000 :
    {
        _data_start = .;
        *(.data*)
        _edata = .;
    }

    .bss  0x80005500 :
    {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _end = .;
    }
}
