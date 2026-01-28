ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
SECTIONS
{
    . = 0x00400000; /* Load address (Virtual Address) */
    .asm : ALIGN(4096)
    {
        *start.S.o(.asm)
        *(.asm)
    }
    .text : ALIGN(4096)
    {
        *(.text)
    }
    .rodata : ALIGN(4096)
    {
        *(.rodata)
    }
    .data : ALIGN(4096)
    {
        *(.data)
    }
    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
    }
}