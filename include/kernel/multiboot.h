#ifndef _KERNEL_MULTIBOOT_H
#define _KERNEL_MULTIBOOT_H

#include <types.h>

// Multiboot2 info structure (simplified for framebuffer support)
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t mb_framebuffer_addr;
    uint32_t mb_framebuffer_pitch;
    uint32_t mb_framebuffer_width;
    uint32_t mb_framebuffer_height;
    uint32_t mb_framebuffer_bpp;
};

// Multiboot info pointer and magic number
extern uint32_t mb_magic;
extern struct multiboot_info* mb_info_addr;

#endif // _KERNEL_MULTIBOOT_H
