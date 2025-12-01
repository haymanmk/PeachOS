.code32
.section .asm, "ax", @progbits  # Define .asm section as executable code
.global _start

_start:
loop_start:
    jmp loop_start  # Infinite loop