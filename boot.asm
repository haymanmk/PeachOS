ORG 0x7C00
BITS 16

start:
    mov si, message ; Load address of message into SI
    call print      ; Call print function
    jmp $

print:
    mov bx, 0x00;
.loop:
    lodsb              ; Load byte at DS:SI into AL and increment SI
    cmp al, 0         ; Check for null terminator
    je .done          ; If null, we're done
    call print_char   ; Print the character in AL
    jmp .loop         ; Repeat for next character
.done:
    ret

print_char:
    mov ah, 0x0E        ; BIOS teletype function
    int 0x10            ; Call BIOS video interrupt
    ret

message:
    db 'Hello, World!', 0

times 510-($-$$) db 0 ; Fill the rest of the boot sector with zeros
dw 0xAA55             ; Boot sector signature