#include <stddef.h>
#include <stdint.h>
#include "include/drivers/serial.h"
#include "include/multiboot.h"
#include "include/arch/x86/multiboot2.h"
#include "include/memory/pmm.h"

#define FRAME_SIZE 4096u
#define MAX_PHYS_MEM_BYTES (128u*1024u*1024u)
#define MAX_FRAMES (MAX_PHYS_MEM_BYTES / FRAME_SIZE)

static uint32_t total_frames = MAX_FRAMES;
static uint32_t used_frames = 0;
static uint32_t highest_frame_index = 0;
static uint32_t bitmap[(MAX_FRAMES + 31) / 32];

static inline void set_frame(uint32_t frame_index){ bitmap[frame_index >> 5] |= (1u << (frame_index & 31)); }
static inline void clear_frame(uint32_t frame_index){ bitmap[frame_index >> 5] &= ~(1u << (frame_index & 31)); }
static inline int test_frame(uint32_t frame_index){ return bitmap[frame_index >> 5] & (1u << (frame_index & 31)); }

static void reserve_range(uint32_t start_addr, uint32_t end_addr){
    if (end_addr <= start_addr) return;
    uint32_t start_frame = start_addr / FRAME_SIZE;
    uint32_t end_frame = (end_addr + FRAME_SIZE - 1) / FRAME_SIZE;
    for (uint32_t f = start_frame; f < end_frame; ++f){
        if (!test_frame(f)) { set_frame(f); used_frames++; }
    }
}

// Initialize PMM with memory map information
void pmm_init(uint32_t mem_upper_kb, uint32_t kernel_start, uint32_t kernel_end, struct multiboot_info* mbi) {
    // Clear bitmap: mark all as used initially
    for (size_t i = 0; i < (sizeof(bitmap)/sizeof(bitmap[0])); ++i) bitmap[i] = 0xFFFFFFFFu;

    // Determine highest available frame via mmap
    highest_frame_index = 0;
    if (mbi->flags & (1 << 6)) { // Check if memory map is valid
        struct multiboot_tag *tag = (struct multiboot_tag *) (mbi + 1); // Start after the multiboot_info struct
        while (tag->type != 0) { // End tag
            if (tag->type == 6) { // Memory map tag
                struct multiboot_tag_mmap *mmap_tag = (struct multiboot_tag_mmap *) tag;
                struct multiboot_mmap_entry *entry = mmap_tag->entries;
                uint32_t entry_count = (mmap_tag->size - sizeof(struct multiboot_tag_mmap)) / mmap_tag->entry_size;

                for (uint32_t i = 0; i < entry_count; i++) {
                    if (entry[i].type == 1) { // Available
                        uint64_t region_end = entry[i].addr + entry[i].len;
                        uint32_t region_end32 = (region_end > 0xFFFFFFFFu) ? 0xFFFFFFFFu : (uint32_t)region_end;
                        uint32_t region_start32 = (entry[i].addr > 0xFFFFFFFFu) ? 0xFFFFFFFFu : (uint32_t)entry[i].addr;
                        if (region_end32 > region_start32) {
                            uint32_t end_frame = region_end32 / FRAME_SIZE;
                            if (end_frame > highest_frame_index) highest_frame_index = end_frame;
                            // Mark these frames free
                            uint32_t start_frame = region_start32 / FRAME_SIZE;
                            for (uint32_t f = start_frame; f < end_frame; ++f) {
                                clear_frame(f);
                            }
                        }
                    }
                }
                break; // Only process the first memory map tag
            }
            tag = (struct multiboot_tag *) ((uint8_t *) tag + ((tag->size + 7) & ~7)); // Align to 8 bytes
        }
    } else {
        // Fallback using mem_upper (in KB above 1MB)
        uint32_t total_kb = 1024 + mem_upper_kb;
        uint32_t total_bytes = total_kb * 1024u;
        highest_frame_index = (total_bytes / FRAME_SIZE);
        // Mark frames [0, highest_frame_index) free
        for (uint32_t f = 0; f < highest_frame_index && f < MAX_FRAMES; ++f) clear_frame(f);
    }

    if (highest_frame_index > MAX_FRAMES) highest_frame_index = MAX_FRAMES;
    total_frames = highest_frame_index;

    // Reserve low memory (first 1MB)
    reserve_range(0, 0x100000u);
    // Reserve kernel image
    pmm_mark_used_region(kernel_start, kernel_end - kernel_start);
}

// Basic PMM initialization without memory map
void pmm_init_basic(uint32_t mem_upper_kb, uint32_t kernel_start, uint32_t kernel_end) {
    // Clear bitmap: mark all as used initially
    for (size_t i = 0; i < (sizeof(bitmap)/sizeof(bitmap[0])); ++i) bitmap[i] = 0xFFFFFFFFu;
    
    // Calculate total memory
    uint32_t total_mem_kb = 1024 + mem_upper_kb; // First MB + extended memory
    uint32_t total_frames = (total_mem_kb * 1024) / FRAME_SIZE;
    if (total_frames > MAX_FRAMES) total_frames = MAX_FRAMES;
    
    // Mark all frames as free initially
    for (uint32_t i = 0; i < total_frames; i++) {
        clear_frame(i);
    }
    
    // Mark kernel memory as used
    pmm_mark_used_region(0, 0x100000); // First 1MB
    pmm_mark_used_region(kernel_start, kernel_end - kernel_start);
}

// Default PMM initialization with fixed memory size
void pmm_init_default(uint32_t mem_upper_kb) {
    // Use a default 64MB if memory size is not provided
    if (mem_upper_kb == 0) {
        mem_upper_kb = 63 * 1024; // 64MB - 1MB
    }
    pmm_init_basic(mem_upper_kb, 0, 0);
}

// Mark a region of memory as used
void pmm_mark_used_region(uint32_t base, uint32_t size) {
    if (size == 0) return;
    uint32_t start_frame = base / FRAME_SIZE;
    uint32_t end_frame = (base + size + FRAME_SIZE - 1) / FRAME_SIZE;
    
    for (uint32_t i = start_frame; i < end_frame; i++) {
        if (!test_frame(i)) {
            set_frame(i);
            used_frames++;
        }
    }
}

uint32_t pmm_alloc_frame(void){
    for (uint32_t f = 0; f < total_frames; ++f){
        if (!test_frame(f)){
            set_frame(f);
            used_frames++;
            return f * FRAME_SIZE;
        }
    }
    return 0; // out of memory
}

void pmm_free_frame(uint32_t frame_addr){
    uint32_t f = frame_addr / FRAME_SIZE;
    if (f < total_frames && test_frame(f)){
        clear_frame(f);
        used_frames--;
    }
} 