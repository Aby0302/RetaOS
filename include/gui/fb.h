#pragma once
#include <stdint.h>

// Minimal framebuffer info shared between kernel and userspace
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;  // bytes per scanline
    uint32_t bpp;    // bits per pixel
} fb_info_t;

