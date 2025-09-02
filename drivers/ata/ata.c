#include "include/arch/x86/io.h"
#include "include/drivers/ata.h"
#include "include/kernel/block.h"
#include "include/drivers/serial.h"
#include <stdint.h>
#include <stddef.h>

// Primary IDE (Master) only
#define ATA_IO_BASE   0x1F0
#define ATA_CTRL_BASE 0x3F6

// IO ports
#define ATA_REG_DATA      (ATA_IO_BASE + 0)
#define ATA_REG_ERROR     (ATA_IO_BASE + 1)
#define ATA_REG_SECCNT    (ATA_IO_BASE + 2)
#define ATA_REG_LBA0      (ATA_IO_BASE + 3)
#define ATA_REG_LBA1      (ATA_IO_BASE + 4)
#define ATA_REG_LBA2      (ATA_IO_BASE + 5)
#define ATA_REG_HDDEVSEL  (ATA_IO_BASE + 6)
#define ATA_REG_STATUS    (ATA_IO_BASE + 7)
#define ATA_REG_COMMAND   (ATA_IO_BASE + 7)

#define ATA_REG_DEVCTRL   (ATA_CTRL_BASE + 0)

// Commands
#define ATA_CMD_IDENTIFY  0xEC
#define ATA_CMD_READ_SECT 0x20

// Status bits
#define ATA_SR_BSY  0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DF   0x20
#define ATA_SR_DSC  0x10
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01

static uint32_t g_hda_sectors = 0;
static block_dev_t g_hda;

static int ata_poll(int check_drq){
    // Wait for BSY to clear
    for (int i=0; i<100000; ++i){
        uint8_t st = inb(ATA_REG_STATUS);
        if (!(st & ATA_SR_BSY)){
            if (check_drq){
                if (st & ATA_SR_ERR) return -1;
                if (st & ATA_SR_DF) return -2;
                if (st & ATA_SR_DRQ) return 0;
            } else {
                return 0;
            }
        }
    }
    return -3; // timeout
}

static void ata_select_master(void){
    outb(ATA_REG_HDDEVSEL, 0xE0); // 0xE0: master, LBA mode
    io_wait(); io_wait();
}

static int ata_identify_drive(uint16_t* idbuf){
    ata_select_master();
    outb(ATA_REG_SECCNT, 0);
    outb(ATA_REG_LBA0, 0);
    outb(ATA_REG_LBA1, 0);
    outb(ATA_REG_LBA2, 0);
    outb(ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    if (ata_poll(0) < 0) return -1;

    uint8_t st = inb(ATA_REG_STATUS);
    if (st == 0) return -2; // no device

    // Wait for DRQ
    for(;;){
        st = inb(ATA_REG_STATUS);
        if (st & ATA_SR_ERR) return -3;
        if (st & ATA_SR_DF) return -4;
        if (st & ATA_SR_DRQ) break;
    }

    // Read 256 words
    insw(ATA_REG_DATA, idbuf, 256);
    return 0;
}

int ata_read28_lba(block_dev_t* dev, uint32_t lba, uint32_t count, void* buf){
    if (count == 0) return 0;
    uint8_t* out = (uint8_t*)buf;
    for (uint32_t n = 0; n < count; ++n){
        uint32_t l = lba + n;
        // Setup LBA28 registers
        outb(ATA_REG_HDDEVSEL, (uint8_t)(0xE0 | ((l >> 24) & 0x0F)));
        outb(ATA_REG_SECCNT, 1);
        outb(ATA_REG_LBA0, (uint8_t)(l & 0xFF));
        outb(ATA_REG_LBA1, (uint8_t)((l >> 8) & 0xFF));
        outb(ATA_REG_LBA2, (uint8_t)((l >> 16) & 0xFF));
        outb(ATA_REG_COMMAND, ATA_CMD_READ_SECT);

        int rc = ata_poll(1);
        if (rc < 0) return rc;

        // Read 256 words (512 bytes)
        insw(ATA_REG_DATA, out, 256);
        out += 512;
    }
    return 0;
}

void ata_init(void){
    // Soft reset: set nIEN=0, SRST=1 then 0 (optional). For simplicity, skip SRST here.
    uint16_t id[256];
    int rc = ata_identify_drive(id);
    if (rc == 0){
        // total LBA28 sectors: words 60-61 (little endian)
        uint32_t lba28 = ((uint32_t)id[61] << 16) | id[60];
        g_hda_sectors = lba28;
        g_hda.name = "hda";
        g_hda.sector_size = 512;
        g_hda.sectors = g_hda_sectors;
        g_hda.read = ata_read28_lba;
        g_hda.drv = NULL;
        blk_register(&g_hda);
        serial_write("[ATA] hda identified, sectors=\n");
        serial_write_dec(g_hda_sectors);
        serial_write("\n");
    } else {
        serial_write("[ATA] no drive or identify failed\n");
    }
}
