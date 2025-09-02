#ifndef RETAOS_BLOCK_H
#define RETAOS_BLOCK_H

#include <stdint.h>

struct block_dev;
typedef int (*blk_read_fn)(struct block_dev* dev, uint32_t lba, uint32_t count, void* buf);

typedef struct block_dev {
    const char* name;      // e.g. "hda"
    uint32_t sector_size;  // bytes (typically 512)
    uint32_t sectors;      // total sectors (if known)
    blk_read_fn read;      // read callback (PIO)
    void* drv;             // driver-private
} block_dev_t;

// Registry
int blk_register(block_dev_t* dev);
int blk_count(void);
block_dev_t* blk_get(int index);
block_dev_t* blk_find(const char* name);

// Convenience
int blk_read(block_dev_t* dev, uint32_t lba, uint32_t count, void* buf);
int blk_read_byname(const char* name, uint32_t lba, uint32_t count, void* buf);

#endif
