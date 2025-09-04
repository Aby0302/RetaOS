#include <stdint.h>
#include <stddef.h>
#include <drivers/serial.h>
#include <arch/x86/paging.h>
#include <kernel/types.h>

// Kernel page directory (placeholder - will be set during initialization)
page_directory_t* kernel_directory = NULL;

// Simple memcpy implementation
void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

// Simple memset implementation
void* memset(void* dest, int c, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    for (size_t i = 0; i < n; i++) {
        d[i] = (unsigned char)c;
    }
    return dest;
}

// kprintf is now implemented in kernel.c

// Simple vfs_size implementation (placeholder)
// vfs_size now implemented in vfs.c



// Simple map_page implementation
void map_page(page_directory_t* dir, void* virt, void* phys, int flags) {
    // TODO: Implement proper page mapping
    (void)dir;
    (void)virt;
    (void)phys;
    (void)flags;
    // For now, do nothing
}
