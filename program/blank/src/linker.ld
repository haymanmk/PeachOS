ENTRY(_start)
OUTPUT_FORMAT(binary)
SECTIONS
{
    . = 0x00400000; /* Load address (Virtual Address) */
    .asm : ALIGN(4096)
    {
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