#ifndef _FAT32_VFS_H
#define _FAT32_VFS_H

#include <kernel/vfs.h>
#include "fat32.h"

// Function prototypes
vfs_node_t* fat32_mount(const char* device);
int fat32_umount(vfs_node_t* node);

// FAT32 specific functions (exported for internal use)
ssize_t fat32_read_file_data(uint32_t first_cluster, uint32_t offset, void* buffer, size_t size);
int fat32_readdir_index(uint32_t dir_cluster, uint32_t index, fat32_dir_t* entry);
int fat32_find_entry(uint32_t dir_cluster, const char* name, fat32_dir_t* entry);

#endif // _FAT32_VFS_H
