#include <stddef.h>
#include <stdint.h>
#include "include/drivers/serial.h"
#include "include/memory/heap.h"

#define HEAP_SIZE (64*1024)
static uint8_t heap_area[HEAP_SIZE];
static size_t heap_offset = 0;

void kheap_init(void){
    heap_offset = 0;
    serial_write("[Heap] 64KB bump allocator ready.\n");
}

void* kmalloc(size_t size){
    if (size == 0) return (void*)0;
    // Align to 8 bytes
    size = (size + 7) & ~((size_t)7);
    if (heap_offset + size > HEAP_SIZE) return (void*)0;
    void* ptr = &heap_area[heap_offset];
    heap_offset += size;
    return ptr;
}

void kfree(void* ptr){ (void)ptr; /* no-op */ } 