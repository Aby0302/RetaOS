#pragma once
#include <stdint.h>

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

// Multiboot information structure (subset)
typedef struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type; // 1 = available, other = reserved
} __attribute__((packed)) multiboot_mmap_entry_t;

typedef struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} __attribute__((packed)) multiboot_info_t; 