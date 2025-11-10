.code32
.section .asm

.global paging_load_directory
.global paging_enable

# Load the page directory base address into CR3
# The page directory address should be passed in stack as the first argument
paging_load_directory:
    push %ebp # Save base pointer
    mov %esp, %ebp # Set base pointer
    mov 8(%ebp), %eax # Get the page directory address from stack
    mov %eax, %cr3 # Load the page directory address into CR3
    pop %ebp # Restore base pointer
    ret

# Enable paging by setting the PG bit in CR0
paging_enable:
    push %ebp # Save base pointer
    mov %esp, %ebp # Set base pointer
    mov %cr0, %eax # Move CR0 to EAX
    or $0x80000000, %eax # Set the PG bit (bit 31)
    mov %eax, %cr0 # Move modified EAX back to CR0
    pop %ebp # Restore base pointer
    ret