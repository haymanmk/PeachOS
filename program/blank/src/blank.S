.code32
.section .asm, "ax", @progbits  # Define .asm section as executable code
.global _start

_start:
    # Calling a system call to test ISR 0x80h handler with command number 0 (sum)
    movl $0, %eax        # Command number 0 (sum)
    int $0x80          # Trigger software interrupt 0x80

loop_start:
    jmp loop_start  # Infinite loop