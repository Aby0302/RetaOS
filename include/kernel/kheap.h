#ifndef _KERNEL_KHEAP_H
#define _KERNEL_KHEAP_H

#include <stddef.h>
#include <stdint.h>

// Kernel heap başlangıç adresi
#define KHEAP_START 0xC0000000
#define KHEAP_INITIAL_SIZE 0x100000  // 1MB başlangıç boyutu

// Hizalama değeri
#define KHEAP_ALIGN 0x1000

// Bellek blok başlığı
typedef struct heap_block {
    struct heap_block* next;
    size_t size;
    uint8_t used;
} heap_block_t;

// Kernel heap yapısı
typedef struct {
    uint32_t start_address;
    uint32_t end_address;
    uint32_t max_address;
    uint8_t supervisor;
    uint8_t readonly;
} heap_t;

// Fonksiyon prototipleri
void kheap_init(void);
void* kmalloc(size_t size);
void* kmalloc_a(size_t size);
void* kmalloc_ap(size_t size, uint32_t* phys);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t size);
void* kcalloc(size_t num, size_t size);

#endif // _KERNEL_KHEAP_H
