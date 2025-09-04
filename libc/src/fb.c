#include <retaos/fb.h>
#include <sys/syscall.h>

int fb_getinfo(fb_info_t* out) {
    if (!out) return -1;
    return SYSCALL2(SYS_FB_GETINFO, (long)out, sizeof(*out));
}

int fb_fill(uint32_t rgb) {
    return SYSCALL1(SYS_FB_FILL, rgb);
}

int fb_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t rgb) {
    return SYSCALL5(SYS_FB_RECT, x, y, w, h, rgb);
}

