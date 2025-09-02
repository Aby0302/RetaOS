#ifndef RETAOS_ATA_H
#define RETAOS_ATA_H

#include <stdint.h>
#include "../kernel/block.h"

void ata_init(void);
int ata_read28_lba(block_dev_t* dev, uint32_t lba, uint32_t count, void* buf);

#endif
