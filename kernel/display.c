// Display initialization and simple framebuffer helpers
#include <stdint.h>
#include <stddef.h>
#include <include/gui/display.h>
#include <arch/x86/multiboot2.h>

// Global framebuffer info
struct video_mode current_mode;

// Multiboot2 tag iterator helper
static inline const struct multiboot_tag* next_tag(const struct multiboot_tag* tag) {
    uint32_t size = (tag->size + 7) & ~7; // 8-byte align
    return (const struct multiboot_tag*)((const uint8_t*)tag + size);
}

void display_init(unsigned long magic, uintptr_t mbi_addr) {
    // Default: no framebuffer
    current_mode.width = current_mode.height = current_mode.pitch = current_mode.bpp = 0;
    current_mode.addr = 0;

    // Accept both common constants seen in the codebase
    const uint32_t MB2_MAGIC_A = 0x36d76289; // canonical multiboot2 bootloader magic
    const uint32_t MB2_MAGIC_B = MULTIBOOT2_BOOTLOADER_MAGIC; // header defines 0x2BADB002 in this tree

    if (magic != MB2_MAGIC_A && magic != MB2_MAGIC_B) {
        return; // Not multiboot2, leave defaults
    }

    const struct multiboot_tag* tag;
    const struct multiboot_tag* tags = (const struct multiboot_tag*)(mbi_addr + 8); // skip total_size + reserved

    for (tag = tags; tag && tag->type != MULTIBOOT_TAG_TYPE_END; tag = next_tag(tag)) {
        if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
            const struct multiboot_tag_framebuffer* fb = (const struct multiboot_tag_framebuffer*)tag;
            uint64_t fb_addr = fb->framebuffer_addr;
            current_mode.width  = fb->framebuffer_width;
            current_mode.height = fb->framebuffer_height;
            current_mode.pitch  = fb->framebuffer_pitch;
            current_mode.bpp    = fb->framebuffer_bpp;
            current_mode.addr   = (uintptr_t)fb_addr;
            break;
        }
    }
}

static inline void put_pixel_linear(uint8_t* fb, uint32_t pitch, uint8_t bpp, uint32_t x, uint32_t y, uint32_t rgb) {
    uint32_t off = y * pitch + x * (bpp/8);
    if (bpp == 32) {
        fb[off+0] = (rgb >> 0) & 0xFF;   // B
        fb[off+1] = (rgb >> 8) & 0xFF;   // G
        fb[off+2] = (rgb >> 16) & 0xFF;  // R
        fb[off+3] = 0xFF;
    } else if (bpp == 24) {
        fb[off+0] = (rgb >> 0) & 0xFF;
        fb[off+1] = (rgb >> 8) & 0xFF;
        fb[off+2] = (rgb >> 16) & 0xFF;
    }
}

void display_set_mode(uint32_t width, uint32_t height, uint32_t bpp) {
    // This does not actually program hardware; it only updates the structure.
    current_mode.width  = width;
    current_mode.height = height;
    current_mode.bpp    = bpp;
    current_mode.pitch  = width * (bpp / 8);
    // keep addr as-is; will be set by multiboot info when available
}

void display_set_pixel(uint32_t x, uint32_t y, uint32_t rgb) {
    if (!current_mode.addr) return;
    if (x >= current_mode.width || y >= current_mode.height) return;
    uint8_t* fb = (uint8_t*)current_mode.addr;
    put_pixel_linear(fb, current_mode.pitch, current_mode.bpp, x, y, rgb);
}

void display_clear(uint32_t rgb) {
    if (!current_mode.addr) return;
    volatile uint8_t* fb = (volatile uint8_t*)current_mode.addr;
    for (uint32_t y = 0; y < current_mode.height; ++y) {
        for (uint32_t x = 0; x < current_mode.width; ++x) {
            put_pixel_linear((uint8_t*)fb, current_mode.pitch, current_mode.bpp, x, y, rgb);
        }
    }
}

void display_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t rgb) {
    if (!current_mode.addr) return;
    uint32_t x2 = (x + w > current_mode.width)  ? current_mode.width  : x + w;
    uint32_t y2 = (y + h > current_mode.height) ? current_mode.height : y + h;
    volatile uint8_t* fb = (volatile uint8_t*)current_mode.addr;
    for (uint32_t yy = y; yy < y2; ++yy) {
        for (uint32_t xx = x; xx < x2; ++xx) {
            put_pixel_linear((uint8_t*)fb, current_mode.pitch, current_mode.bpp, xx, yy, rgb);
        }
    }
}