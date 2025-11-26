.section .asm

.global tss_load
tss_load:
    push %ebp           // Save base pointer
    mov %esp, %ebp      // Set new base pointer

    // Load the TSS segment selector into the task register
    mov 8(%ebp), %ax    // Get TSS segment selector from argument
    ltr %ax             // Load task register with TSS selector

    mov %ebp, %esp      // Restore stack pointer
    pop %ebp            // Restore base pointer
    ret                 // Return from function