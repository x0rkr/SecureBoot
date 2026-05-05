[BITS 16]
[ORG 0x7C00]

STAGE2_LOAD_ADDR equ 0x8000
STAGE2_SECTORS   equ 16

start:
    cli
    xor  ax, ax
    mov  ds, ax
    mov  es, ax
    mov  ss, ax
    mov  sp, 0x7C00
    sti

    mov  [boot_drive], dl
    mov  si, msg_banner
    call print_string

enable_a20:
    mov  ax, 0x2401
    int  0x15
    jnc  a20_done
    in   al, 0x92
    or   al, 0x02
    and  al, 0xFE
    out  0x92, al

a20_done:
    mov  si, msg_a20
    call print_string

load_stage2:
    mov  si, msg_loading
    call print_string
    mov  dl, [boot_drive]
    mov  bx, STAGE2_LOAD_ADDR
    mov  dh, 0
    mov  ch, 0
    mov  cl, 2
    mov  al, STAGE2_SECTORS
.retry:
    mov  ah, 0x02
    int  0x13
    jnc  .disk_ok
    xor  ax, ax       ; Reset disk
    int  0x13
    dec  byte [retry_count]
    jnz  .retry
    jmp  disk_error
.disk_ok:
    mov  si, msg_ok
    call print_string
    jmp  STAGE2_LOAD_ADDR

print_string:
    mov  ah, 0x0E
.loop:
    lodsb
    test al, al
    jz   .done
    int  0x10
    jmp  .loop
.done:
    ret

disk_error:
    mov  si, msg_disk_err
    call print_string
.halt:
    hlt
    jmp  .halt

boot_drive   db 0
retry_count  db 3
msg_banner   db 0x0D, 0x0A, "[SecureBoot] Stage 1 v1.0", 0x0D, 0x0A, \
               "  Developed by radi0aktiv | github.com/radi0aktiv", 0x0D, 0x0A, 0
msg_a20      db "[*] A20 enabled", 0x0D, 0x0A, 0
msg_loading  db "[*] Loading Stage 2...", 0x0D, 0x0A, 0
msg_ok       db "[+] Jumping to Stage 2...", 0x0D, 0x0A, 0
msg_disk_err db "[!] Disk error. Halted.", 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55