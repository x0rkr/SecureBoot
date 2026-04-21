#!/bin/bash
# ===========================================================
# SecureBoot Project — QEMU Run Script
# Dev: x0rkr | github.com/x0rkr
# ===========================================================

BOOT_IMG="build/boot.bin"

if [ ! -f "$BOOT_IMG" ]; then
    echo "[!] Boot image not found. Run 'make' first."
    exit 1
fi

echo ""
echo "  [*] Starting SecureBoot in QEMU..."
echo "  [*] Press Ctrl+Alt+G to release mouse"
echo "  [*] Close the QEMU window to quit"
echo ""

qemu-system-x86_64 \
    -drive format=raw,file=$BOOT_IMG,index=0,media=disk \
    -m 32M \
    -no-reboot \
    -no-shutdown \
    -vga std \
    -display gtk
