#pragma once
#include <stdint.h>

void pmm_init(uint32_t mem_upper_kb, uint32_t kernel_start, uint32_t kernel_end, struct multiboot_info* mbi);
void pmm_init_basic(uint32_t mem_upper_kb, uint32_t kernel_start, uint32_t kernel_end);
void pmm_init_default(uint32_t mem_upper_kb);
void pmm_mark_used_region(uint32_t base, uint32_t size);
uint32_t pmm_alloc_frame(void);
void pmm_free_frame(uint32_t frame_addr);