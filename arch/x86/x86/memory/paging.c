#include <stdint.h>
#include "include/drivers/serial.h"
#include "include/memory/pmm.h"
#include "include/arch/x86/paging.h"

#define PAGE_PRESENT 0x001
#define PAGE_RW      0x002
#define PAGE_USER    0x004

static uint32_t __attribute__((aligned(4096))) page_directory[1024];
static uint32_t __attribute__((aligned(4096))) first_page_table[1024];
// Map the kernel heap high region (e.g., 0xC0000000..)
static uint32_t __attribute__((aligned(4096))) heap_page_table[1024];

extern void* kmalloc(unsigned long size);

static inline void invlpg(void* addr){ __asm__ __volatile__("invlpg (%0)" :: "r"(addr) : "memory"); }

void paging_init(void){
    // Zero PD and PT
    for (int i = 0; i < 1024; ++i){ page_directory[i] = 0; first_page_table[i] = 0; }

    // Identity-map first 4MB as user-accessible so simple userspace can run
    for (int i = 0; i < 1024; ++i){
        first_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }
    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW | PAGE_USER;

    // Map a 1MB kernel heap at virtual 0xC0000000 to physical 4MB..5MB
    // PDE index for 0xC0000000 is 0x300
    for (int i = 0; i < 256; ++i) { // 256 * 4KB = 1MB
        heap_page_table[i] = ((0x400000 + (i * 0x1000)) & ~0xFFFu) | PAGE_PRESENT | PAGE_RW; // supervisor RW
    }
    for (int i = 256; i < 1024; ++i) heap_page_table[i] = 0; // clear rest
    page_directory[0x300] = ((uint32_t)heap_page_table) | PAGE_PRESENT | PAGE_RW;

    // Load CR3
    __asm__ __volatile__("mov %0, %%cr3" :: "r"(page_directory));

    // Enable paging (set PG bit in CR0)
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u; // PG
    __asm__ __volatile__("mov %0, %%cr0" :: "r"(cr0));

    serial_write("[Paging] Enabled with identity map (4MB) + heap at 0xC0000000 (1MB).\n");
}

// Map a single 4KB page at 'virt' to 'phys' with RW kernel perms
void paging_map_page(uint32_t virt, uint32_t phys){
    uint32_t pd_idx = (virt >> 22) & 0x3FF;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;
    uint32_t pde = page_directory[pd_idx];
    uint32_t* pt;
    if (!(pde & PAGE_PRESENT)){
        // allocate a new page table
        pt = (uint32_t*)kmalloc(4096);
        // zero it
        for (int i=0;i<1024;i++) pt[i]=0;
        page_directory[pd_idx] = ((uint32_t)pt) | PAGE_PRESENT | PAGE_RW; // supervisor RW
        // reload CR3 to flush TLB for new PT
        __asm__ __volatile__("mov %0, %%cr3" :: "r"(page_directory));
    } else {
        pt = (uint32_t*)(pde & ~0xFFFu);
    }
    pt[pt_idx] = (phys & ~0xFFFu) | PAGE_PRESENT | PAGE_RW;
    invlpg((void*)virt);
}

// (no exports)
