#include <stddef.h>
#include <stdint.h>
#include "include/drivers/serial.h"
#include "include/multiboot.h"
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

void pmm_init(uint32_t mem_upper_kb, uint32_t kernel_start, uint32_t kernel_end, uint32_t mmap_addr, uint32_t mmap_len){
    // Clear bitmap: mark all as used initially
    for (size_t i = 0; i < (sizeof(bitmap)/sizeof(bitmap[0])); ++i) bitmap[i] = 0xFFFFFFFFu;

    // Determine highest available frame via mmap
    highest_frame_index = 0;
    if (mmap_len && mmap_addr){
        uint32_t ptr = mmap_addr;
        uint32_t end = mmap_addr + mmap_len;
        while (ptr < end){
            multiboot_mmap_entry_t* e = (multiboot_mmap_entry_t*)(uintptr_t)ptr;
            if (e->type == 1){ // available
                uint64_t region_end = e->addr + e->len;
                uint32_t region_end32 = (region_end > 0xFFFFFFFFu) ? 0xFFFFFFFFu : (uint32_t)region_end;
                uint32_t region_start32 = (e->addr > 0xFFFFFFFFu) ? 0xFFFFFFFFu : (uint32_t)e->addr;
                if (region_end32 > region_start32){
                    uint32_t end_frame = region_end32 / FRAME_SIZE;
                    if (end_frame > highest_frame_index) highest_frame_index = end_frame;
                    // Mark these frames free
                    uint32_t start_frame = region_start32 / FRAME_SIZE;
                    for (uint32_t f = start_frame; f < end_frame; ++f){
                        clear_frame(f);
                    }
                }
            }
            ptr += e->size + sizeof(e->size);
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
    reserve_range(kernel_start, kernel_end);

    serial_write("[PMM] frames="); serial_write_dec(total_frames); serial_write(" used="); serial_write_dec(used_frames); serial_write("\n");
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