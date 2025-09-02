#ifndef _IDE_H
#define _IDE_H

#include <stdint.h>
#include <stdbool.h>

// IDE device types
#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

// IDE commands
#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_READ_PIO_EXT    0x24
#define ATA_CMD_READ_DMA        0xC8
#define ATA_CMD_READ_DMA_EXT    0x25
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_WRITE_PIO_EXT   0x34
#define ATA_CMD_WRITE_DMA       0xCA
#define ATA_CMD_WRITE_DMA_EXT   0x35
#define ATA_CMD_CACHE_FLUSH     0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY        0xEC

// IDE registers
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07

// Status register bits
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

// Identify data offsets
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

// IDE device structure
typedef struct {
    uint8_t  present;    // 0 = no device, 1 = device present
    uint8_t  channel;    // 0 = primary, 1 = secondary
    uint8_t  drive;      // 0 = master, 1 = slave
    uint16_t type;       // 0 = ATA, 1 = ATAPI
    uint16_t signature;  // Drive signature
    uint16_t capabilities; // Features
    uint32_t command_sets; // Command sets supported
    uint32_t size;       // Size in sectors
    char     model[41];  // Model in string
} ide_device_t;

// Function declarations
void ide_init(void);
void ide_wait_irq(void);
uint8_t ide_ata_access(uint8_t direction, uint8_t drive, uint32_t lba, uint8_t numsects, uint16_t selector, uint32_t edi);
void ide_read_sectors(uint8_t drive, uint32_t lba, uint8_t numsects, uint16_t es, uint32_t edi);
void ide_write_sectors(uint8_t drive, uint32_t lba, uint8_t numsects, uint16_t es, uint32_t edi);
void ide_irq_handler(void);

#endif // _IDE_H
