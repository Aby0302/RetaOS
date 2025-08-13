#pragma once
#include <stdint.h>

void pmm_init(uint32_t mem_upper_kb, uint32_t kernel_start, uint32_t kernel_end, uint32_t mmap_addr, uint32_t mmap_len);
uint32_t pmm_alloc_frame(void);
void pmm_free_frame(uint32_t frame_addr); 