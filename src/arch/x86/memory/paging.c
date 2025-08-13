#include <stdint.h>
#include "include/drivers/serial.h"
#include "include/memory/pmm.h"
#include "include/arch/x86/paging.h"

#define PAGE_PRESENT 0x001
#define PAGE_RW      0x002
#define PAGE_USER    0x004

static uint32_t __attribute__((aligned(4096))) page_directory[1024];
static uint32_t __attribute__((aligned(4096))) first_page_table[1024];

void paging_init(void){
    // Zero PD and PT
    for (int i = 0; i < 1024; ++i){ page_directory[i] = 0; first_page_table[i] = 0; }

    // Identity-map first 4MB
    for (int i = 0; i < 1024; ++i){
        first_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_RW;
    }
    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW;

    // Load CR3
    __asm__ __volatile__("mov %0, %%cr3" :: "r"(page_directory));

    // Enable paging (set PG bit in CR0)
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u; // PG
    __asm__ __volatile__("mov %0, %%cr0" :: "r"(cr0));

    serial_write("[Paging] Enabled with identity map (4MB).\n");
} 