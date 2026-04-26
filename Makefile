# ===========================================================
# SecureBoot Project — Makefile
# Dev: x0rkr | github.com/x0rkr
# ===========================================================

AS       = nasm
CC       = gcc
LD       = ld

ASFLAGS  = -f bin
CCFLAGS  = -m32 -ffreestanding -fno-stack-protector \
           -fno-builtin -nostdlib -nostdinc -fno-pic \
           -fno-pie -O2 -Wall
LDFLAGS  = -m elf_i386 -T linker.ld --oformat binary

BUILD    = build
SRC      = src

.PHONY: all clean run

all: $(BUILD)/boot.bin

# Assemble Stage 1 (MBR)
$(BUILD)/stage1.bin: $(SRC)/boot/stage1.asm
	$(AS) $(ASFLAGS) $< -o $@

# Assemble Stage 2 entry
$(BUILD)/stage2_entry.o: $(SRC)/boot/stage2_entry.asm
	$(AS) -f elf32 $< -o $@

# Compile C sources
$(BUILD)/screen.o: $(SRC)/drivers/screen.c
	$(CC) $(CCFLAGS) -c $< -o $@

$(BUILD)/disk.o: $(SRC)/drivers/disk.c
	$(CC) $(CCFLAGS) -c $< -o $@

$(BUILD)/crypto.o: $(SRC)/security/crypto.c
	$(CC) $(CCFLAGS) -c $< -o $@

$(BUILD)/integrity.o: $(SRC)/security/integrity.c
	$(CC) $(CCFLAGS) -c $< -o $@

$(BUILD)/main.o: $(SRC)/kernel/main.c
	$(CC) $(CCFLAGS) -c $< -o $@

$(BUILD)/shell.o: $(SRC)/kernel/shell.c
	$(CC) $(CCFLAGS) -c $< -o $@

# Link Stage 2 + C objects into binary
$(BUILD)/stage2.bin: $(BUILD)/stage2_entry.o \
                     $(BUILD)/screen.o \
                     $(BUILD)/disk.o \
                     $(BUILD)/crypto.o \
                     $(BUILD)/integrity.o \
                     $(BUILD)/shell.o \
                     $(BUILD)/main.o
	$(LD) $(LDFLAGS) $^ -o $@

# Combine Stage1 + Stage2 into final boot image
$(BUILD)/boot.bin: $(BUILD)/stage1.bin $(BUILD)/stage2.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=$(BUILD)/stage1.bin of=$@ conv=notrunc bs=512 seek=0
	dd if=$(BUILD)/stage2.bin of=$@ conv=notrunc bs=512 seek=1
	@echo ""
	@echo "  [+] Build SUCCESS: $(BUILD)/boot.bin"
	@echo "  [+] Dev: x0rkr | github.com/x0rkr"
	@echo ""
# Run in QEMU
run: $(BUILD)/boot.bin
	qemu-system-x86_64 -drive format=raw,file=$(BUILD)/boot.bin

# Clean build artifacts
clean:
	rm -f $(BUILD)/*.bin $(BUILD)/*.o
	@echo "  [+] Build directory cleaned."

