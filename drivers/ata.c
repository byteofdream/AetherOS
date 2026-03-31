#include "ata.h"
#include "port.h"

#define ATA_IO 0x1F0
#define ATA_CTRL 0x3F6

static bool ata_wait(void) {
    for (int i = 0; i < 100000; ++i) {
        uint8_t status = inb(ATA_IO + 7);
        if (!(status & 0x80) && (status & 0x08)) {
            return true;
        }
    }
    return false;
}

bool ata_init(void) {
    outb(ATA_CTRL, 0);
    outb(ATA_IO + 6, 0xA0);
    outb(ATA_IO + 2, 0);
    outb(ATA_IO + 3, 0);
    outb(ATA_IO + 4, 0);
    outb(ATA_IO + 5, 0);
    outb(ATA_IO + 7, 0xEC);
    uint8_t status = inb(ATA_IO + 7);
    if (status == 0) {
        return false;
    }
    if (!ata_wait()) {
        return false;
    }
    for (int i = 0; i < 256; ++i) {
        (void)inw(ATA_IO);
    }
    return true;
}

bool ata_read28(uint32_t lba, uint8_t sector_count, void *buffer) {
    uint16_t *ptr = buffer;
    outb(ATA_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO + 2, sector_count);
    outb(ATA_IO + 3, (uint8_t)lba);
    outb(ATA_IO + 4, (uint8_t)(lba >> 8));
    outb(ATA_IO + 5, (uint8_t)(lba >> 16));
    outb(ATA_IO + 7, 0x20);

    for (uint8_t sector = 0; sector < sector_count; ++sector) {
        if (!ata_wait()) {
            return false;
        }
        for (int i = 0; i < 256; ++i) {
            *ptr++ = inw(ATA_IO);
        }
    }
    return true;
}

bool ata_write28(uint32_t lba, uint8_t sector_count, const void *buffer) {
    const uint16_t *ptr = buffer;
    outb(ATA_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO + 2, sector_count);
    outb(ATA_IO + 3, (uint8_t)lba);
    outb(ATA_IO + 4, (uint8_t)(lba >> 8));
    outb(ATA_IO + 5, (uint8_t)(lba >> 16));
    outb(ATA_IO + 7, 0x30);

    for (uint8_t sector = 0; sector < sector_count; ++sector) {
        if (!ata_wait()) {
            return false;
        }
        for (int i = 0; i < 256; ++i) {
            outw(ATA_IO, *ptr++);
        }
    }

    outb(ATA_IO + 7, 0xE7);
    return ata_wait();
}
