#include <kernel/kheap.h>
#include <kernel/pmm.h>
#include <kernel/console.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Global kernel heap
heap_t *kheap = 0;

// Basit bir bellek yöneticisi için blok listesi
static heap_block_t* first_block = NULL;
static uint32_t kheap_start = 0;
static uint32_t kheap_end = 0;
static uint32_t kheap_max = 0;
static uint32_t kheap_inited = 0;

// Hizalama fonksiyonu
static uint32_t align_up(uint32_t addr, uint32_t align) {
    return (addr + align - 1) & ~(align - 1);
}

// Kernel heap'i başlat
void kheap_init(void) {
    if (kheap_inited) return;
    
    // Fiziksel bellek yöneticisini başlat
    pmm_init();
    
    // Kernel heap için bellek ayır
    kheap_start = KHEAP_START;
    kheap_end = kheap_start + KHEAP_INITIAL_SIZE;
    kheap_max = kheap_start + 0xFFFFF; // 1MB maksimum
    
    // İlk bloğu oluştur
    first_block = (heap_block_t*)kheap_start;
    first_block->size = kheap_end - kheap_start - sizeof(heap_block_t);
    first_block->used = 0;
    first_block->next = NULL;
    
    kheap_inited = 1;
    //kprintf("[kheap] Kernel heap initialized at 0x%x\n", kheap_start);
}

// Bellek ayırma fonksiyonu
void* kmalloc(size_t size) {
    if (!kheap_inited) kheap_init();
    
    // En küçük blok boyutu
    if (size < sizeof(heap_block_t*)) {
        size = sizeof(heap_block_t*);
    }
    
    // Blok arama
    heap_block_t* block = first_block;
    heap_block_t* prev = NULL;
    
    while (block) {
        if (!block->used && block->size >= size) {
            // Blok bulundu, bölmeye çalış
            if (block->size > size + sizeof(heap_block_t) + 4) {
                // Yeni blok oluştur
                heap_block_t* new_block = (heap_block_t*)((uint8_t*)block + sizeof(heap_block_t) + size);
                new_block->size = block->size - size - sizeof(heap_block_t);
                new_block->used = 0;
                new_block->next = block->next;
                
                block->size = size;
                block->next = new_block;
            }
            
            block->used = 1;
            return (void*)((uint8_t*)block + sizeof(heap_block_t));
        }
        
        prev = block;
        block = block->next;
    }
    
    // Yeterli alan yok, genişlet
    uint32_t new_size = align_up(size + sizeof(heap_block_t), KHEAP_ALIGN);
    if (kheap_end + new_size > kheap_max) {
        //kprintf("[kheap] Out of memory!\n");
        return NULL;
    }
    
    // Yeni blok oluştur
    heap_block_t* new_block = (heap_block_t*)kheap_end;
    new_block->size = size;
    new_block->used = 1;
    new_block->next = NULL;
    
    if (prev) {
        prev->next = new_block;
    } else {
        first_block = new_block;
    }
    
    kheap_end += new_size;
    
    return (void*)((uint8_t*)new_block + sizeof(heap_block_t));
}

// Bellek serbest bırakma fonksiyonu
void kfree(void* ptr) {
    if (!ptr || (uint32_t)ptr < kheap_start || (uint32_t)ptr >= kheap_end) {
        return;
    }
    
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    block->used = 0;
    
    // Ardışık boş blokları birleştir
    heap_block_t* current = first_block;
    while (current && current->next) {
        if (!current->used && !current->next->used) {
            current->size += current->next->size + sizeof(heap_block_t);
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

// Yeniden boyutlandırma fonksiyonu
void* krealloc(void* ptr, size_t size) {
    if (!ptr) {
        return kmalloc(size);
    }
    
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    if (block->size >= size) {
        return ptr; // Mevcut blok yeterli
    }
    
    // Yeni bellek bloğu ayır
    void* new_ptr = kmalloc(size);
    if (!new_ptr) {
        return NULL;
    }
    
    // Veriyi kopyala
    memcpy(new_ptr, ptr, block->size);
    
    // Eski bloğu serbest bırak
    kfree(ptr);
    
    return new_ptr;
}

// Sıfırlanmış bellek ayırma fonksiyonu
void* kcalloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = kmalloc(total);
    
    if (ptr) {
        memset(ptr, 0, total);
    }
    
    return ptr;
}

// Hizalanmış bellek ayırma fonksiyonları
void* kmalloc_a(size_t size) {
    return kmalloc(align_up(size, KHEAP_ALIGN));
}

void* kmalloc_ap(size_t size, uint32_t* phys) {
    void* addr = kmalloc_a(size);
    if (phys) {
        *phys = (uint32_t)addr - 0xC0000000; // Sanal adresten fiziksel adrese dönüşüm
    }
    return addr;
}
