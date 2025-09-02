#include <stdlib.h>
#include <string.h>

static void *heap_end = 0;

extern void *_sbrk(intptr_t increment) {
    void *prev_heap_end;
    __asm__ volatile ("int $0x80" : "=a" (prev_heap_end) : "a" (10), "b" (increment));
    return prev_heap_end;
}

void *malloc(size_t size) {
    if (size == 0) return NULL;
    
    // Simple first-fit allocation
    struct block_meta *block;
    void *request = _sbrk(size + sizeof(struct block_meta));
    if (request == (void*) -1) return NULL;
    
    block = (struct block_meta*)request;
    block->size = size;
    block->free = 0;
    
    return (void*)(block + 1);
}

void free(void *ptr) {
    // Simple implementation - just mark as free
    if (!ptr) return;
    struct block_meta *block = (struct block_meta*)ptr - 1;
    block->free = 1;
}

void *calloc(size_t nmemb, size_t size) {
    void *ptr = malloc(nmemb * size);
    if (ptr) memset(ptr, 0, nmemb * size);
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) { free(ptr); return NULL; }
    
    struct block_meta *block = (struct block_meta*)ptr - 1;
    if (block->size >= size) return ptr;
    
    void *new_ptr = malloc(size);
    if (!new_ptr) return NULL;
    
    memcpy(new_ptr, ptr, block->size);
    free(ptr);
    return new_ptr;
}

// Simple atoi implementation
int atoi(const char *nptr) {
    int res = 0;
    int sign = 1;
    int i = 0;
    
    // Skip whitespace
    while (nptr[i] == ' ') i++;
    
    // Handle sign
    if (nptr[i] == '-') {
        sign = -1;
        i++;
    } else if (nptr[i] == '+') {
        i++;
    }
    
    // Convert number
    while (nptr[i] >= '0' && nptr[i] <= '9') {
        res = res * 10 + (nptr[i] - '0');
        i++;
    }
    
    return sign * res;
}

// Simple exit implementation
void _exit(int status) {
    __asm__ volatile ("int $0x80" : : "a" (1), "b" (status));
    while (1); // Should never reach here
}

// Memory block metadata
struct block_meta {
    size_t size;
    int free;
    struct block_meta *next;
};

// Start with NULL as the head of our free list
static struct block_meta *global_base = NULL;
