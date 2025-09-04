#include <stdio.h>
#include <stdint.h>
#include <retaos/fb.h>

static inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b){ return (r<<16)|(g<<8)|b; }

int main(void) {
    fb_info_t info;
    if (fb_getinfo(&info) != 0 || info.width == 0) {
        printf("GUI: framebuffer not available.\n");
        return 1;
    }

    // Paint desktop background
    fb_fill(rgb(40, 60, 90));

    // Simple taskbar
    fb_rect(0, info.height - 28, info.width, 28, rgb(230,230,230));

    // Draw two demo windows
    fb_rect(40, 40, 320, 200, rgb(250,250,250));
    fb_rect(42, 42, 316, 24, rgb(180,210,255)); // title bar
    fb_rect(400, 120, 260, 160, rgb(245,245,245));
    fb_rect(402, 122, 256, 24, rgb(200,200,255));

    printf("GUI demo running (user-mode). Press 'q' to exit.\n");
    // Basic input loop. Character will echo on text console underneath.
    for (;;) {
        int c = getchar();
        if (c == 'q' || c == 'Q' || c == 27) break; // ESC or q
    }
    return 0;
}
