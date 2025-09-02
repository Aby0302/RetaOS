#ifndef _KERNEL_HEAP_H
#define _KERNEL_HEAP_H

#include <stddef.h>

// Initialize the kernel heap
void heap_init(void);

// Allocate memory from the kernel heap
void* kmalloc(size_t size);

// Free memory allocated from the kernel heap
void kfree(void* ptr);

// Reallocate memory block
void* krealloc(void* ptr, size_t size);

// Get the total amount of free memory in the heap
size_t get_free_memory(void);

// Get the total amount of used memory in the heap
size_t get_used_memory(void);

#endif // _KERNEL_HEAP_H
