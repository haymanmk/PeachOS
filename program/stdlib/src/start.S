.code32
.section .asm, "ax", @progbits  # Define .asm section as executable code
.global _start

.extern main

_start:
    call main
    ret

.section .data
.align 4
message:
    .asciz "I can talk to kernel.\n"