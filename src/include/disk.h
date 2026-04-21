#ifndef DISK_H
#define DISK_H

/* ATA PIO Port Addresses */
#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE_HEAD  0x1F6
#define ATA_STATUS      0x1F7
#define ATA_CMD         0x1F7

/* ATA Status Bits */
#define ATA_SR_BSY      0x80    /* Busy */
#define ATA_SR_DRDY     0x40    /* Drive ready */
#define ATA_SR_DRQ      0x08    /* Data request */
#define ATA_SR_ERR      0x01    /* Error */

/* ATA Commands */
#define ATA_CMD_READ    0x20    /* Read sectors */
#define ATA_CMD_WRITE   0x30    /* Write sectors */

/* Sector size in bytes */
#define SECTOR_SIZE     512

/* Function Prototypes */
int disk_read(unsigned int lba, unsigned char count, unsigned char *buffer);
int disk_write(unsigned int lba, unsigned char count, unsigned char *buffer);

#endif

