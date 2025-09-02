#pragma once
#include <stdint.h>

// Mount a ustar initrd from a block device into the VFS as root (/) .
// Reads up to max_sectors sectors starting at start_lba from device named dev_name.
// A hard cap (max_bytes_cap) is used to avoid excessive memory usage.
// Returns 0 on success, <0 on error.
int initrd_mount_from_block(const char* dev_name, uint32_t start_lba, uint32_t max_sectors, uint32_t max_bytes_cap);
