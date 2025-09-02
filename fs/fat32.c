#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "../include/kernel/vfs.h"
#include "../include/kernel/fs.h"
#include "../include/kernel/heap.h"
#include "../include/kernel/console_utils.h"
#include "../include/drivers/ata.h"
#include "../include/kernel/block.h"
#include "fat32.h"

// Forward declarations
static uint32_t get_first_sector_of_cluster(uint32_t cluster);
// Using system strcasecmp instead of our own implementation
static int read_sectors(uint32_t lba, uint8_t num_sectors, void *buffer);

// Define VFS flags for backward compatibility
#ifndef VFS_FILE
#define VFS_FILE      0x01
#define VFS_DIRECTORY 0x02
#endif



// FAT32 private data structure
typedef struct {
    uint32_t first_cluster;
    uint32_t size;
    uint32_t pos;
} fat32_file_priv_t;

// Forward declarations
static int is_eof_cluster(uint32_t cluster);
static uint32_t get_first_sector_of_cluster(uint32_t cluster);
static int read_sectors(uint32_t lba, uint8_t num_sectors, void *buffer);

// Our own implementation of strcasecmp
int strcasecmp(const char *s1, const char *s2) {
    const unsigned char *us1 = (const unsigned char *)s1;
    const unsigned char *us2 = (const unsigned char *)s2;
    
    while (tolower(*us1) == tolower(*us2)) {
        if (*us1 == '\0') {
            return 0;
        }
        us1++;
        us2++;
    }
    
    return tolower(*us1) - tolower(*us2);
}
int fat32_init(void);

// File system state - make some accessible to VFS layer
fat32_boot_sector_t g_boot_sector;
uint32_t* g_fat = NULL;
static uint32_t g_total_clusters = 0;
static uint32_t g_fat_begin_lba = 0;
static uint32_t g_cluster_begin_lba = 0;
int g_initialized = 0;

// Check if cluster is end of file
static int is_eof_cluster(uint32_t cluster) {
    return (cluster >= 0x0FFFFFF8);
}

// Get first sector of a cluster
static uint32_t get_first_sector_of_cluster(uint32_t cluster) {
    return g_cluster_begin_lba + ((cluster - 2) * g_boot_sector.sectors_per_cluster);
}

// Case-insensitive string comparison
// Using system strcasecmp instead of custom implementation
// Custom implementation removed to avoid conflict with system declaration

// Read sectors from disk
static int read_sectors(uint32_t lba, uint8_t num_sectors, void *buffer) {
    // Use the ATA driver to read sectors
    // Get the first block device (hda)
    block_dev_t* dev = blk_find("hda");
    if (!dev) {
        return -1;
    }
    return ata_read28_lba(dev, lba, num_sectors, buffer);
}

// Initialize FAT32 filesystem
int fat32_init(void) {
    if (g_initialized) {
        return 0;
    }

    // Read boot sector
    if (read_sectors(0, 1, (void*)&g_boot_sector) != 0) {
        console_printf("Failed to read boot sector\n");
        return -1;
    }
    
    // Verify FAT32 signature
    if (strncmp((char*)g_boot_sector.fs_type, "FAT32", 5) != 0) {
        console_printf("Not a FAT32 filesystem\n");
        return -1;
    }
    
    // Calculate important FAT32 locations
    g_fat_begin_lba = g_boot_sector.reserved_sector_count;
    g_cluster_begin_lba = g_fat_begin_lba + (g_boot_sector.num_fats * g_boot_sector.fat_size_32);
    
    // Calculate total clusters
    uint32_t data_sectors = g_boot_sector.total_sectors_32 - g_cluster_begin_lba;
    g_total_clusters = data_sectors / g_boot_sector.sectors_per_cluster;
    
    // Allocate memory for FAT
    uint32_t fat_size = g_boot_sector.fat_size_32 * g_boot_sector.bytes_per_sector;
    g_fat = (uint32_t*)kmalloc(fat_size);
    
    if (!g_fat) {
        console_printf("Failed to allocate memory for FAT\n");
        return -1;
    }
    
    // Read the FAT in chunks if needed
    uint32_t sectors_remaining = g_boot_sector.fat_size_32;
    uint32_t current_lba = g_fat_begin_lba;
    uint8_t* fat_ptr = (uint8_t*)g_fat;
    
    while (sectors_remaining > 0) {
        uint8_t sectors_to_read = (sectors_remaining > 255) ? 255 : sectors_remaining;
        
        if (read_sectors(current_lba, sectors_to_read, fat_ptr) != 0) {
            console_printf("Failed to read FAT at LBA %u\n", current_lba);
            kfree(g_fat);
            return -1;
        }
        
        sectors_remaining -= sectors_to_read;
        current_lba += sectors_to_read;
        fat_ptr += sectors_to_read * g_boot_sector.bytes_per_sector;
    }
    
    g_initialized = 1;
    console_printf("FAT32 filesystem initialized\n");
    console_printf("Volume label: %s\n", g_boot_sector.volume_label);
    console_printf("Total clusters: %u\n", g_total_clusters);
    
    return 0;
}



// Find a file in a directory
int fat32_find_entry(uint32_t dir_cluster, const char* name, fat32_dir_t* entry) {
    if (!g_initialized || !name || !entry) {
        return -EINVAL;
    }

    uint8_t sector_buffer[512];
    uint32_t current_cluster = dir_cluster;

    while (!is_eof_cluster(current_cluster) && current_cluster != 0) {
        uint32_t first_sector = get_first_sector_of_cluster(current_cluster);

        // Read all sectors in the cluster
        for (uint32_t s = 0; s < g_boot_sector.sectors_per_cluster; s++) {
            if (read_sectors(first_sector + s, 1, sector_buffer) != 0) {
                return -EIO;
            }

            fat32_dir_entry_t* dir_entries = (fat32_dir_entry_t*)sector_buffer;

            // Process each entry in the sector
            for (int i = 0; i < 16; i++) {  // 16 entries per sector (512/32)
                fat32_dir_entry_t* de = &dir_entries[i];

                // Skip deleted or empty entries
                if (de->name[0] == 0x00) {
                    continue;  // End of directory
                }
                if (de->name[0] == 0xE5) {
                    continue;  // Deleted entry
                }

                // Skip volume label, hidden, and system files
                if ((de->attributes & ATTR_VOLUME_ID) ||
                    (de->attributes & ATTR_HIDDEN) ||
                    (de->attributes & ATTR_SYSTEM)) {
                    continue;
                }

                // Convert 8.3 name to a more readable format
                char entry_name[13];
                int pos = 0;

                // Copy name (8 characters)
                for (int j = 0; j < 8; j++) {
                    if (de->name[j] != ' ') {
                        entry_name[pos++] = tolower(de->name[j]);
                    }
                }

                // Add dot if there's an extension
                if (de->name[8] != ' ') {
                    entry_name[pos++] = '.';

                    // Copy extension (3 characters)
                    for (int j = 8; j < 11; j++) {
                        if (de->name[j] != ' ') {
                            entry_name[pos++] = tolower(de->name[j]);
                        }
                    }
                }

                entry_name[pos] = '\0';

                // Compare names
                if (strcasecmp(entry_name, name) == 0) {
                    // Found a match - fill in the fat32_dir_t structure
                    strncpy(entry->name, entry_name, sizeof(entry->name) - 1);
                    entry->name[sizeof(entry->name) - 1] = '\0';
                    entry->size = de->file_size;
                    entry->cluster = (de->first_cluster_high << 16) | de->first_cluster_low;
                    entry->attributes = de->attributes;
                    return 0;  // Success
                }
            }
        }

        // Move to next cluster in the chain
        current_cluster = g_fat[current_cluster];
    }

    return -ENOENT;
}

// Read file data from clusters
ssize_t fat32_read_file_data(uint32_t first_cluster, uint32_t offset, void* buffer, size_t size) {
    if (!g_initialized || !buffer) {
        return -EINVAL;
    }

    if (size == 0) {
        return 0;
    }

    // Cache these values to avoid repeated memory access
    uint32_t bytes_per_sector = g_boot_sector.bytes_per_sector;
    uint32_t sectors_per_cluster = g_boot_sector.sectors_per_cluster;
    uint32_t bytes_per_cluster = sectors_per_cluster * bytes_per_sector;
    uint32_t bytes_remaining = size;
    uint32_t bytes_read = 0;
    uint32_t current_cluster = first_cluster;
    uint32_t current_offset = 0;

    // Allocate buffer for reading clusters
    uint8_t* cluster_buffer = (uint8_t*)kmalloc(bytes_per_cluster);
    if (!cluster_buffer) {
        console_printf("FAT32: Failed to allocate cluster buffer\n");
        return -ENOMEM;
    }

    // Skip to the starting cluster
    while (current_offset + bytes_per_cluster <= offset && !is_eof_cluster(current_cluster)) {
        current_cluster = g_fat[current_cluster];
        current_offset += bytes_per_cluster;
    }

    // Calculate initial cluster offset
    uint32_t cluster_offset = offset - current_offset;

    // Read data cluster by cluster
    while (bytes_remaining > 0 && !is_eof_cluster(current_cluster)) {
        uint32_t sector = get_first_sector_of_cluster(current_cluster);

        // Read the entire cluster
        if (read_sectors(sector, sectors_per_cluster, cluster_buffer) != 0) {
            console_printf("FAT32: Error reading cluster %u at sector %u\n",
                         current_cluster, sector);
            kfree(cluster_buffer);
            return -EIO;
        }

        // Calculate how much to read from this cluster
        uint32_t to_read = bytes_per_cluster - cluster_offset;
        if (to_read > bytes_remaining) {
            to_read = bytes_remaining;
        }

        // Copy data from cluster buffer to user buffer
        memcpy((uint8_t*)buffer + bytes_read, cluster_buffer + cluster_offset, to_read);

        // Update counters and move to next cluster
        bytes_read += to_read;
        bytes_remaining -= to_read;
        current_cluster = g_fat[current_cluster];
        cluster_offset = 0;  // For subsequent clusters, start from beginning
    }

    // Free the cluster buffer
    kfree(cluster_buffer);

    return bytes_read;
}

// Read directory entry by index
int fat32_readdir_index(uint32_t dir_cluster, uint32_t index, fat32_dir_t* entry) {
    if (!g_initialized || !entry) {
        return -EINVAL;
    }

    uint8_t sector_buffer[512];
    uint32_t current_cluster = dir_cluster;
    uint32_t entries_found = 0;

    while (!is_eof_cluster(current_cluster) && current_cluster != 0) {
        uint32_t first_sector = get_first_sector_of_cluster(current_cluster);

        // Read all sectors in the cluster
        for (uint32_t s = 0; s < g_boot_sector.sectors_per_cluster; s++) {
            if (read_sectors(first_sector + s, 1, sector_buffer) != 0) {
                return -EIO;
            }

            fat32_dir_entry_t* dir_entries = (fat32_dir_entry_t*)sector_buffer;

            // Process each entry in the sector
            for (int i = 0; i < 16; i++) {  // 16 entries per sector (512/32)
                fat32_dir_entry_t* de = &dir_entries[i];

                // Skip deleted or empty entries
                if (de->name[0] == 0x00) {
                    continue;  // End of directory
                }
                if (de->name[0] == 0xE5) {
                    continue;  // Deleted entry
                }

                // Skip volume label, hidden, and system files
                if ((de->attributes & ATTR_VOLUME_ID) ||
                    (de->attributes & ATTR_HIDDEN) ||
                    (de->attributes & ATTR_SYSTEM)) {
                    continue;
                }

                // Check if this is the requested index
                if (entries_found == index) {
                    // Convert 8.3 name to a more readable format
                    int pos = 0;

                    // Copy name (8 characters)
                    for (int j = 0; j < 8; j++) {
                        if (de->name[j] != ' ') {
                            entry->name[pos++] = tolower(de->name[j]);
                        }
                    }

                    // Add dot if there's an extension
                    if (de->name[8] != ' ') {
                        entry->name[pos++] = '.';

                        // Copy extension (3 characters)
                        for (int j = 8; j < 11; j++) {
                            if (de->name[j] != ' ') {
                                entry->name[pos++] = tolower(de->name[j]);
                            }
                        }
                    }

                    entry->name[pos] = '\0';
                    entry->size = de->file_size;
                    entry->cluster = (de->first_cluster_high << 16) | de->first_cluster_low;
                    entry->attributes = de->attributes;

                    return 0;  // Success
                }

                entries_found++;
            }
        }

        // Move to next cluster in the chain
        current_cluster = g_fat[current_cluster];
    }

    return -ENOENT;  // Index not found
}



// Cleanup FAT32 resources
void fat32_cleanup(void) {
    if (g_fat) {
        kfree(g_fat);
        g_fat = NULL;
    }
    
    g_initialized = 0;
}
