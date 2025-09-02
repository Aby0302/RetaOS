#ifndef _KERNEL_PMM_H
#define _KERNEL_PMM_H

#include <stdint.h>
#include <stddef.h>

// Sayfa boyutu (4KB)
#define PMM_PAGE_SIZE 0x1000

// Fiziksel bellek yöneticisini başlat
void pmm_init(void);

// Fiziksel bellek ayır
void* pmm_alloc(void);

// Fiziksel bellek serbest bırak
void pmm_free(void* addr);

// Kullanılabilir bellek miktarını al
uint32_t pmm_get_free_memory(void);

// Kullanılan bellek miktarını al
uint32_t pmm_get_used_memory(void);

// Toplam bellek miktarını al
uint32_t pmm_get_total_memory(void);

#endif // _KERNEL_PMM_H
