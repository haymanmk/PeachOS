.section .asm
.code32

.global inb
.global inw
.global outb
.global outw

inb:
    push %ebp
    mov %esp, %ebp

    xor %eax, %eax
    mov 8(%ebp), %dx
    inb %dx, %al

    mov %ebp, %esp
    pop %ebp
    ret

inw:
    push %ebp
    mov %esp, %ebp

    xor %eax, %eax
    mov 8(%ebp), %dx
    inw %dx, %ax

    mov %ebp, %esp
    pop %ebp
    ret

outb:
    push %ebp
    mov %esp, %ebp

    mov 8(%ebp), %dx    # Get port from first argument
    mov 12(%ebp), %al   # Get value from second argument
    outb %al, %dx

    mov %ebp, %esp
    pop %ebp
    ret

outw:
    push %ebp
    mov %esp, %ebp

    mov 8(%ebp), %dx    # Get port from first argument
    mov 12(%ebp), %ax   # Get value from second argument
    outw %ax, %dx

    mov %ebp, %esp
    pop %ebp
    ret
