/*
 * SecureBoot Project — Linker Script
 * Dev: x0rkr | github.com/x0rkr
 *
 * Ensures correct memory layout:
 * Stage 2 loads at 0x8000
 * C code follows without overlapping BIOS Data Area
 */

OUTPUT_FORMAT("binary")

ENTRY(start16)

SECTIONS {
    /* Stage 2 entry point — must match STAGE2_LOAD_ADDR in stage1.asm */
    . = 0x8000;

    .text : {
        /* Stage 2 ASM entry goes first */
        *stage2_entry.o (.text)
        /* Then all C code */
        *(.text)
    }

    .data : {
        *(.data)
        *(.rodata)
    }

    .bss : {
        *(.bss)
        *(COMMON)
    }

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
    }
}

