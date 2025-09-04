#pragma once
#include <stdint.h>
#include <gui/fb.h>

// Userspace wrappers for framebuffer syscalls
int fb_getinfo(fb_info_t* out);
int fb_fill(uint32_t rgb);
int fb_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t rgb);

