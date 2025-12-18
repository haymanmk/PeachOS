.code32
.section .asm, "ax", @progbits  # Define .asm section as executable code
.global _start

_start:
    # Get a character from the keyboard
    # call get_keyboard_char
    pushl %eax          # Push the character onto the stack as an argument
    movl $3, %eax        # Command number 3: Put character to terminal
    # int $0x80          # Trigger software interrupt 0x80
    addl $4, %esp      # Clean up the stack (1 arguments * 4 bytes each)

_loop_start:
    jmp _loop_start     # Repeat indefinitely

get_keyboard_char:
    movl $2, %eax        # Command number 2: Get keyboard character
    int $0x80          # Trigger software interrupt 0x80
    # Retry getting character until a valid character is received
    cmpb $0, %al        # Check if the character is null
    je get_keyboard_char # If null, try again
    ret

.section .data
.align 4
message:
    .asciz "I can talk to kernel.\n"