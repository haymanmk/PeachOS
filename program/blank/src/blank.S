.code32
.section .asm, "ax", @progbits  # Define .asm section as executable code
.global _start

_start:
    # Calling a system call to test ISR 0x80h handler with command number 0 (sum)
    push $5             # First argument
    push $10            # Second argument
    movl $0, %eax        # Command number 0 (sum)
    int $0x80          # Trigger software interrupt 0x80
    addl $8, %esp      # Clean up the stack (2 arguments * 4 bytes each)

    .loop_start:
        jmp .loop_start  # Infinite loop