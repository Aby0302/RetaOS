#include "fs/fat32.h"
#include "fs/fat32.h"
#include "../include/kernel/ide.h"
#include "../include/kernel/console.h"
#include "../include/kernel/console_utils.h"
#include "../include/kernel/heap.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

// Global variables
static fat32_boot_sector_t g_boot_sector;
static uint32_t g_fat_begin_lba;
static uint32_t g_cluster_begin_lba;
static uint32_t g_root_dir_sectors;
static uint32_t g_total_clusters;
static uint32_t* g_fat = NULL;
static int g_initialized = 0;

// Helper functions
static uint32_t get_cluster_from_entry(uint32_t entry) {
    return entry & 0x0FFFFFFF;
}

static int is_eof_cluster(uint32_t cluster) {
    return (cluster >= 0x0FFFFFF8) || (cluster == 0x0FFFFFFF);
}

static uint32_t get_next_cluster(uint32_t current_cluster) {
    if (!g_fat || current_cluster < 2 || current_cluster >= g_total_clusters + 2) {
        return 0xFFFFFFFF;
    }
    return get_cluster_from_entry(g_fat[current_cluster]);
}

static uint32_t get_first_sector_of_cluster(uint32_t cluster) {
    if (cluster < 2 || cluster >= g_total_clusters + 2) {
        return 0;
    }
    return g_cluster_begin_lba + ((cluster - 2) * g_boot_sector.sectors_per_cluster);
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
        console_print("Not a FAT32 filesystem\n");
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
    console_print("FAT32 filesystem initialized\n");
    console_print("Volume label: %s\n", g_boot_sector.volume_label);
    console_print("Total clusters: %d\n", g_total_clusters);
    
    return 0;
}

// Read a directory entry by index
int fat32_readdir_index(uint32_t dir_cluster, uint32_t index, fat32_dir_t* entry) {
    if (!g_initialized || !entry) {
        return -EINVAL;
    }
    
    uint8_t sector_buffer[512];
    uint32_t current_cluster = dir_cluster;
    uint32_t entries_per_sector = 512 / sizeof(fat32_dir_entry_t);
    uint32_t entry_count = 0;
    
    while (!is_eof_cluster(current_cluster) && current_cluster != 0) {
        uint32_t first_sector = get_first_sector_of_cluster(current_cluster);
        
        // Read all sectors in the cluster
        for (uint32_t s = 0; s < g_boot_sector.sectors_per_cluster; s++) {
            if (read_sectors(first_sector + s, 1, sector_buffer) != 0) {
                return -EIO;
            }
            
            fat32_dir_entry_t* dir_entries = (fat32_dir_entry_t*)sector_buffer;
            
            // Process each entry in the sector
            for (uint32_t i = 0; i < entries_per_sector; i++) {
                fat32_dir_entry_t* de = &dir_entries[i];
                
                // Skip deleted or empty entries
                if (de->name[0] == 0x00) {
                    return -ENOENT;  // No more entries
                }
                if (de->name[0] == 0xE5) {
                    continue;  // Deleted entry
                }
                if ((de->attributes & ATTR_VOLUME_ID) || 
                    (de->attributes & ATTR_HIDDEN) || 
                    (de->attributes & ATTR_SYSTEM)) {
                    continue;  // Skip special entries
                }
                
                // Check if this is the entry we're looking for
                if (entry_count == index) {
                    // Found our entry
                    memset(entry, 0, sizeof(fat32_dir_t));
                    
                    // Copy name (convert from 8.3 to long name if needed)
                    if (de->name[0] == 0x05) {
                        entry->name[0] = 0xE5;  // Special case for Japanese filenames
                    } else {
                        strncpy(entry->name, (const char*)de->name, 8);
                    }
                    
                    // Add extension if present
                    if (de->name[8] != ' ') {
                        char* dot = strchr(entry->name, '\0');
                        *dot++ = '.';
                        strncpy(dot, (const char*)&de->name[8], 3);
                        dot[3] = '\0';
                    }
                    
                    // Copy other attributes
                    entry->attributes = de->attributes;
                    entry->size = de->file_size;
                    
                    // Get first cluster number
                    uint16_t hi = de->first_cluster_high;
                    uint16_t lo = de->first_cluster_low;
                    entry->cluster = (hi << 16) | lo;
                    
                    return 0;
                }
                
                entry_count++;
            }
        }
        
        // Move to next cluster in the chain
        current_cluster = get_next_cluster(current_cluster);
    }
    
    return -ENOENT;  // Entry not found
}

// Find a directory entry by name
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
            for (uint32_t i = 0; i < 16; i++) {  // 16 entries per sector (512/32)
                fat32_dir_entry_t* de = &dir_entries[i];
                
                // Skip deleted or empty entries
                if (de->name[0] == 0x00) {
                    return -ENOENT;  // No more entries
                }
                if (de->name[0] == 0xE5) {
                    continue;  // Deleted entry
                }
                if ((de->attributes & ATTR_VOLUME_ID) || 
                    (de->attributes & ATTR_HIDDEN) || 
                    (de->attributes & ATTR_SYSTEM)) {
                    continue;  // Skip special entries
                }
                
                // Convert 8.3 name to a string we can compare
                char entry_name[13];
                int pos = 0;
                
                // Copy name part (8 chars)
                for (int j = 0; j < 8 && de->name[j] != ' '; j++) {
                    entry_name[pos++] = de->name[j];
                }
                
                // Add extension if present
                if (de->name[8] != ' ') {
                    entry_name[pos++] = '.';
                    for (int j = 8; j < 11 && de->name[j] != ' '; j++) {
                        entry_name[pos++] = de->name[j];
                    }
                }
                entry_name[pos] = '\0';
                
                // Compare names (case-insensitive)
                if (strcasecmp(entry_name, name) == 0) {
                    // Found our entry
                    memset(entry, 0, sizeof(fat32_dir_t));
                    strncpy(entry->name, entry_name, sizeof(entry->name) - 1);
                    entry->attributes = de->attributes;
                    entry->size = de->file_size;
                    
                    // Get first cluster number
                    uint16_t hi = de->first_cluster_high;
                    uint16_t lo = de->first_cluster_low;
                    entry->cluster = (hi << 16) | lo;
                    
                    return 0;
                }
            }
        }
        
        // Move to next cluster in the chain
        current_cluster = get_next_cluster(current_cluster);
    }
    
    return -ENOENT;  // Entry not found
}

// Read data from a file
ssize_t fat32_read_file_data(uint32_t first_cluster, uint32_t offset, void* buffer, size_t size) {
    if (!g_initialized || !buffer || first_cluster < 2) {
        return -EINVAL;
    }
    
    uint8_t* buf = (uint8_t*)buffer;
    uint32_t bytes_read = 0;
    uint32_t cluster = first_cluster;
    uint32_t bytes_per_cluster = g_boot_sector.bytes_per_sector * g_boot_sector.sectors_per_cluster;
    
    // Skip to the starting cluster
    uint32_t start_cluster = offset / bytes_per_cluster;
    for (uint32_t i = 0; i < start_cluster; i++) {
        cluster = get_next_cluster(cluster);
        if (is_eof_cluster(cluster) || cluster == 0) {
            return 0;  // Offset beyond file end
        }
    }
    
    // Calculate starting position within the cluster
    uint32_t cluster_offset = offset % bytes_per_cluster;
    uint32_t remaining_in_cluster = bytes_per_cluster - cluster_offset;
    
    // Read data cluster by cluster
    while (size > 0 && !is_eof_cluster(cluster) && cluster != 0) {
        uint32_t to_read = (size < remaining_in_cluster) ? size : remaining_in_cluster;
        uint32_t sector = get_first_sector_of_cluster(cluster) + (cluster_offset / g_boot_sector.bytes_per_sector);
        uint32_t sector_offset = cluster_offset % g_boot_sector.bytes_per_sector;
        
        if (to_read > 0) {
            // Read the sector(s) containing our data
            uint32_t sectors_to_read = (sector_offset + to_read + g_boot_sector.bytes_per_sector - 1) / g_boot_sector.bytes_per_sector;
            uint8_t* temp_buf = kmalloc(sectors_to_read * g_boot_sector.bytes_per_sector);
            
            if (!temp_buf) {
                return -ENOMEM;
            }
            
            if (read_sectors(sector, sectors_to_read, temp_buf) != 0) {
                kfree(temp_buf);
                return -EIO;
            }
            
            // Copy the requested data
            memcpy(buf, temp_buf + sector_offset, to_read);
            kfree(temp_buf);
            
            // Update counters
            buf += to_read;
            bytes_read += to_read;
            size -= to_read;
            
            // Reset for next cluster
            cluster_offset = 0;
            remaining_in_cluster = bytes_per_cluster;
        }
        
        // Move to next cluster
        cluster = get_next_cluster(cluster);
    }
    
    return bytes_read;
}

// Read data from a file by name (legacy function, kept for compatibility)
ssize_t fat32_read_file(const char* filename, void* buffer, size_t size) {
    if (!g_initialized || !filename || !buffer) {
        return -EINVAL;
    }
    
    // Find the file
    fat32_dir_t entry;
    if (fat32_find_entry(g_boot_sector.root_cluster, filename, &entry) != 0) {
        return -ENOENT;
    }
    
    // Read the file data
    return fat32_read_file_data(entry.cluster, 0, buffer, size);
}

// Write data to a file
ssize_t fat32_write_file(const char* filename, const void* buffer, size_t size) {
    if (!g_initialized || !filename || !buffer) {
        return -EINVAL;
    }
    // 2. Allocate clusters if needed
    // 3. Write data cluster by cluster
    
    return -ENOSYS; // Not implemented yet
}

// Create a new file
int fat32_create_file(const char* filename) {
    if (!g_initialized || !filename) {
        return -EINVAL;
    }

    // TODO: Implement file creation
    // 1. Find an empty directory entry
    // 2. Allocate a new cluster
    // 3. Update the directory entry
    
    return -ENOSYS; // Not implemented yet
}

// Delete a file
int fat32_delete_file(const char* filename) {
    if (!g_initialized || !filename) {
        return -EINVAL;
    }

    // TODO: Implement file deletion
    // 1. Find the file in the directory
    // 2. Free all clusters used by the file
    // 3. Mark the directory entry as free
    
    return -ENOSYS; // Not implemented yet
}
