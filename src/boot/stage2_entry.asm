[BITS 16]

global start16

start16:
    cli
    mov  si, msg_stage2
    call print16

    lgdt [gdt_descriptor]

    mov  eax, cr0
    or   eax, 0x1
    mov  cr0, eax

    jmp  0x08:start32

print16:
    mov  ah, 0x0E
.loop:
    lodsb
    test al, al
    jz   .done
    int  0x10
    jmp  .loop
.done:
    ret

gdt_start:
    dq 0x0000000000000000

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

msg_stage2 db "[*] Stage 2 active | Dev: radi0aktiv", 0x0D, 0x0A, 0

[BITS 32]
global start32
start32:
    mov  ax, 0x10
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax
    mov  ss, ax
    mov  esp, 0x90000

    extern kernel_main
    call   kernel_main

.halt:
    hlt
    jmp  .halt