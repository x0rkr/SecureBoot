# SecureBoot — Bare-Metal x86 Authenticated Bootloader

> A fully custom, from-scratch bootloader written in NASM Assembly and C.
> No standard library. No shortcuts. Pure bare-metal.

**Dev:** x0rkr | **GitHub:** [github.com/x0rkr](https://github.com/x0rkr)

---

## What is this?

SecureBoot is a two-stage x86 bootloader that runs before any operating system. It implements a full pre-boot security layer including
password authentication, VM detection, and XOR kernel decryption — all written from scratch at the hardware level.

---

## Features

| Feature | Description |
| :--- | :--- |
| **Stage 1 MBR** | 512-byte entry point, A20 activation, disk loading |
| **Stage 2 Protected Mode** | GDT setup, 16-bit to 32-bit transition |
| **VGA Text Driver** | Direct writes to `0xB8000`, custom `kprintf` |
| **Password Auth** | djb2 hashed password, masked input, 3 attempts |
| **Anti-VM Detection** | CPUID hypervisor checks, timing anomaly detection |
| **XOR Decryption** | Encrypted kernel decrypted in RAM before boot |
| **Secure Wipe** | Sensitive memory zeroed on auth failure |
| **CI/CD** | GitHub Actions auto-builds on every push |

---

## Project Structure

```text
/secure-boot-project
├── Makefile                 # Build automation
├── linker.ld                # Linker script (memory layout)
├── run.sh                   # QEMU test script
├── src/
│   ├── boot/
│   │   ├── stage1.asm       # MBR entry point (512 bytes)
│   │   └── stage2_entry.asm # GDT + Protected Mode switch
│   ├── drivers/
│   │   ├── screen.c         # VGA text mode driver
│   │   └── disk.c           # ATA PIO disk I/O
│   ├── include/             # Header files
│   ├── kernel/
│   │   └── main.c           # Core security engine
│   └── security/
│       ├── crypto.c         # XOR encryption + djb2 hash
│       └── integrity.c      # Anti-VM + timing checks
└── docs/                    # Documentation



Requirements:

sudo apt install nasm gcc make binutils qemu-system-x86

Build
Since Git does not track empty directories, ensure the build/ directory exists before compiling.

Bash
mkdir -p build
make
Note: If you get an error stating unable to open output file build/stage1.bin, it means the build directory is missing. The command above creates it.

Run in QEMU
Bash
./run.sh
Change Boot Password
Generate a new djb2 hash for your password:

Python
python3 -c "
s = 'YourPassword'
h = 5381
for c in s:
    h = ((h << 5) + h + ord(c)) & 0xFFFFFFFF
print(hex(h))
"
Then update src/include/crypto.h:

C
#define PASSWORD_HASH 0xYOURHASH

Rebuild with make clean && make


Memory Map
Address                           Contents
0x7C00                            Stage 1 MBR
0x8000                            Stage 2 entry
0x90000                           Stack
0xB8000                           VGA text buffer
0x100000                          Kernel load address


License

MIT © 2026 x0rkr






