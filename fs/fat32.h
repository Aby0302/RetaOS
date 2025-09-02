#ifndef _FAT32_H
#define _FAT32_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../include/kernel/vfs.h"
#include "../include/kernel/ide.h"

// FAT32 attributes
#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20

// FAT32 directory entry
#pragma pack(push, 1)
typedef struct {
    uint8_t  name[11];           // 8.3 format
    uint8_t  attributes;         // File attributes
    uint8_t  reserved;           // Reserved for Windows NT
    uint8_t  creation_time_tenths; // Tenths of a second
    uint16_t creation_time;      // Creation time
    uint16_t creation_date;      // Creation date
    uint16_t last_access_date;   // Last access date
    uint16_t first_cluster_high; // High 16 bits of first cluster
    uint16_t last_write_time;    // Last write time
    uint16_t last_write_date;    // Last write date
    uint16_t first_cluster_low;  // Low 16 bits of first cluster
    uint32_t file_size;          // File size in bytes
} fat32_dir_entry_t;
#pragma pack(pop)

// FAT32 boot sector
#pragma pack(push, 1)
typedef struct {
    // Common fields for FAT12/16/32
    uint8_t     jump_boot[3];
    uint8_t     oem_name[8];
    uint16_t    bytes_per_sector;
    uint8_t     sectors_per_cluster;
    uint16_t    reserved_sector_count;
    uint8_t     num_fats;
    uint16_t    root_entry_count;
    uint16_t    total_sectors_16;
    uint8_t     media_type;
    uint16_t    fat_size_16;
    uint16_t    sectors_per_track;
    uint16_t    num_heads;
    uint32_t    hidden_sectors;
    uint32_t    total_sectors_32;
    
    // FAT32 specific fields
    uint32_t    fat_size_32;
    uint16_t    ext_flags;
    uint16_t    fs_version;
    uint32_t    root_cluster;
    uint16_t    fs_info;
    uint16_t    backup_boot_sector;
    uint8_t     reserved[12];
    uint8_t     drive_number;
    uint8_t     reserved1;
    uint8_t     boot_signature;
    uint32_t    volume_id;
    uint8_t     volume_label[11];
    uint8_t     fs_type[8];
    uint8_t     boot_code[420];
    uint16_t    boot_signature_55aa;
} fat32_boot_sector_t;
#pragma pack(pop)


// Function declarations
int fat32_init(void);

// File operations
ssize_t fat32_read_file(const char* filename, void* buffer, size_t size);
ssize_t fat32_write_file(const char* filename, const void* buffer, size_t size);
ssize_t fat32_read_file_data(uint32_t first_cluster, uint32_t offset, void* buffer, size_t size);
int fat32_create_file(const char* filename);
int fat32_delete_file(const char* filename);

// Directory operations
typedef struct {
    char name[256];
    uint32_t size;
    uint32_t cluster;
    uint8_t attributes;
} fat32_dir_t;

int fat32_opendir(const char* path, fat32_dir_t* dir);
int fat32_readdir(fat32_dir_t* dir);
int fat32_closedir(fat32_dir_t* dir);
int fat32_readdir_index(uint32_t dir_cluster, uint32_t index, fat32_dir_t* entry);
int fat32_find_entry(uint32_t dir_cluster, const char* name, fat32_dir_t* entry);

// Utility functions
uint32_t fat32_get_free_cluster(void);
int fat32_free_cluster_chain(uint32_t cluster);
int fat32_find_file(const char* path, uint32_t* first_cluster, uint32_t* file_size);

// VFS-related functions
vfs_node_t* fat32_mount(const char* device);
int fat32_umount(vfs_node_t* node);

#endif // _FAT32_H
