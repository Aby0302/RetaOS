#include <kernel/pmm.h>
#include <kernel/console.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Bitmap kullanarak sayfa durumunu takip et
static uint32_t* pmm_bitmap = 0;
static uint32_t pmm_bitmap_size = 0;
static uint32_t pmm_max_blocks = 0;
static uint32_t pmm_used_blocks = 0;
static uint32_t pmm_total_memory = 0;

// Bit işlemleri için yardımcı fonksiyonlar
static void pmm_set_bit(uint32_t bit) {
    pmm_bitmap[bit / 32] |= (1 << (bit % 32));
}

static void pmm_clear_bit(uint32_t bit) {
    pmm_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

static bool pmm_test_bit(uint32_t bit) {
    return pmm_bitmap[bit / 32] & (1 << (bit % 32));
}



// Fiziksel bellek ayır
void* pmm_alloc(void) {
    if (pmm_used_blocks >= pmm_max_blocks) {
        return NULL;
    }
    
    // İlk boş bloğu bul
    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            // Bu kelimede en az bir boş bit var
            for (uint32_t j = 0; j < 32; j++) {
                uint32_t bit = 1 << j;
                if (!(pmm_bitmap[i] & bit)) {
                    // Boş bit bulundu
                    pmm_bitmap[i] |= bit;
                    pmm_used_blocks++;
                    return (void*)((i * 32 + j) * PMM_PAGE_SIZE);
                }
            }
        }
    }
    
    return NULL; // Boş blok bulunamadı
}

// Fiziksel bellek serbest bırak
void pmm_free(void* addr) {
    uint32_t bit = (uint32_t)addr / PMM_PAGE_SIZE;
    
    if (bit >= pmm_max_blocks) {
        return; // Geçersiz adres
    }
    
    if (!pmm_test_bit(bit)) {
        return; // Zaten boş
    }
    
    pmm_clear_bit(bit);
    pmm_used_blocks--;
}

// Kullanılabilir bellek miktarını al
uint32_t pmm_get_free_memory(void) {
    return (pmm_max_blocks - pmm_used_blocks) * PMM_PAGE_SIZE;
}

// Kullanılan bellek miktarını al
uint32_t pmm_get_used_memory(void) {
    return pmm_used_blocks * PMM_PAGE_SIZE;
}

// Toplam bellek miktarını al
uint32_t pmm_get_total_memory(void) {
    return pmm_total_memory;
}
