.code32
.section .asm, "ax", @progbits  # Define .asm section as executable code
.global _start

_start:
    # Calling system call to print a message
    pushl $message     # Push address of the message string
    movl $1, %eax        # Command number 1: Print string
    int $0x80          # Trigger software interrupt 0x80
    addl $4, %esp      # Clean up the stack (1 arguments * 4 bytes each)

    .loop_start:
        jmp .loop_start  # Infinite loop

.section .data
.align 4
message:
    .asciz "I can talk to kernel.\n"