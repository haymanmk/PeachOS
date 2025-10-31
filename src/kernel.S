.code32

.equ CODE_SEG, 0x08
.equ DATA_SEG, 0x10

.global _start
_start:
    # Initialize data segment registers
    movw    $DATA_SEG, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    movl    $0x200000, %ebp  # Set stack pointer to 0x200000
    movl    %ebp, %esp

    # try to do 32-bit operations. The assigned value will be truncated to 16-bit when in real mode
    movl    $0x12345678, %ebx

    jmp    . # Infinite loop
