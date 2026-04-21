#include "../include/disk.h"
#include "../include/screen.h"

/* Port I/O helpers — no stdlib, direct hardware access */
static inline unsigned char inb(unsigned short port) {
    unsigned char val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

/* Wait for disk to be ready */
static void ata_wait(void) {
    while (inb(0x1F7) & 0x80);   /* Wait while BSY bit set */
}

static void ata_wait_drq(void) {
    while (!(inb(0x1F7) & 0x08)); /* Wait for DRQ bit */
}

/* Read sectors from disk using ATA PIO mode
   lba    = Logical Block Address
   count  = number of sectors
   buffer = destination in RAM */
int disk_read(unsigned int lba, unsigned char count, unsigned char *buffer) {
    ata_wait();

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); /* Drive + LBA bits 24-27 */
    outb(0x1F2, count);                         /* Sector count */
    outb(0x1F3, (unsigned char)(lba));          /* LBA low */
    outb(0x1F4, (unsigned char)(lba >> 8));     /* LBA mid */
    outb(0x1F5, (unsigned char)(lba >> 16));    /* LBA high */
    outb(0x1F7, 0x20);                          /* Command: READ SECTORS */

    for (int s = 0; s < count; s++) {
        ata_wait();
        ata_wait_drq();

        /* Read 256 words (512 bytes) per sector */
        unsigned short *buf16 = (unsigned short*)buffer;
        for (int i = 0; i < 256; i++) {
            unsigned short lo = inb(0x1F0);
            unsigned short hi = inb(0x1F0 + 1);
            buf16[i] = lo | (hi << 8);
        }
        buffer += 512;
    }

    return 0;
}

/* Write sectors to disk using ATA PIO mode */
int disk_write(unsigned int lba, unsigned char count, unsigned char *buffer) {
    ata_wait();

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, count);
    outb(0x1F3, (unsigned char)(lba));
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x30);                          /* Command: WRITE SECTORS */

    for (int s = 0; s < count; s++) {
        ata_wait();
        ata_wait_drq();

        unsigned short *buf16 = (unsigned short*)buffer;
        for (int i = 0; i < 256; i++) {
            outb(0x1F0,     (unsigned char)(buf16[i]));
            outb(0x1F0 + 1, (unsigned char)(buf16[i] >> 8));
        }
        buffer += 512;
    }

    return 0;
}

