#ifndef _KERNEL_KALLOC_H
#define _KERNEL_KALLOC_H

#include <stddef.h>

// Kernel memory allocation functions
void* kmalloc(size_t size);
void kfree(void* ptr);
void* kcalloc(size_t nmemb, size_t size);
void* krealloc(void* ptr, size_t size);

// Debug functions
void kalloc_dump(void);

#endif /* _KERNEL_KALLOC_H */
