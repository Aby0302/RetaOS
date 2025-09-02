#include "include/kernel/block.h"
#include <stddef.h>

#define MAX_BLK_DEVS 8
static block_dev_t* g_blk_devs[MAX_BLK_DEVS];
static int g_blk_cnt = 0;

int blk_register(block_dev_t* dev){
    if (!dev || !dev->read || !dev->name) return -1;
    if (g_blk_cnt >= MAX_BLK_DEVS) return -2;
    g_blk_devs[g_blk_cnt++] = dev;
    return 0;
}

int blk_count(void){ return g_blk_cnt; }

block_dev_t* blk_get(int index){
    if (index < 0 || index >= g_blk_cnt) return NULL;
    return g_blk_devs[index];
}

block_dev_t* blk_find(const char* name){
    if (!name) return NULL;
    for (int i=0;i<g_blk_cnt;i++){
        if (!g_blk_devs[i] || !g_blk_devs[i]->name) continue;
        const char* a = g_blk_devs[i]->name; const char* b = name;
        while (*a && *b && *a==*b){ a++; b++; }
        if (*a==0 && *b==0) return g_blk_devs[i];
    }
    return NULL;
}

int blk_read(block_dev_t* dev, uint32_t lba, uint32_t count, void* buf){
    if (!dev || !dev->read || !buf || count==0) return -1;
    return dev->read(dev, lba, count, buf);
}

int blk_read_byname(const char* name, uint32_t lba, uint32_t count, void* buf){
    block_dev_t* d = blk_find(name);
    if (!d) return -1;
    return blk_read(d, lba, count, buf);
}
