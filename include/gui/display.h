#pragma once
#include <stdint.h>
#include <stddef.h>

// Basit video modu bilgisi
struct video_mode {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;     // bytes per scanline
    uint32_t bpp;       // bits per pixel (24 veya 32)
    uintptr_t addr;     // framebuffer'ın DOĞRUDAN erişilecek adresi (virt ya da phys-map)
};

extern struct video_mode current_mode;

// Başlatma (Multiboot2)
void display_init(unsigned long magic, uintptr_t mbi_addr);

// Çizim temel API
void display_set_pixel(uint32_t x, uint32_t y, uint32_t rgb);   // rgb = 0xRRGGBB
void display_clear(uint32_t rgb);
void display_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t rgb);

// (opsiyonel) mod set fallback – gerçek hw modesi yoksa kullanmayın
void display_set_mode(uint32_t width, uint32_t height, uint32_t bpp);
