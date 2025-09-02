#ifndef _KERNEL_CONSOLE_H
#define _KERNEL_CONSOLE_H

#include <stddef.h>
#include <stdint.h>

// Console colors
#define CONSOLE_COLOR_BLACK         0x0
#define CONSOLE_COLOR_BLUE          0x1
#define CONSOLE_COLOR_GREEN         0x2
#define CONSOLE_COLOR_CYAN          0x3
#define CONSOLE_COLOR_RED           0x4
#define CONSOLE_COLOR_MAGENTA       0x5
#define CONSOLE_COLOR_BROWN         0x6
#define CONSOLE_COLOR_LIGHT_GRAY    0x7
#define CONSOLE_COLOR_DARK_GRAY     0x8
#define CONSOLE_COLOR_LIGHT_BLUE    0x9
#define CONSOLE_COLOR_LIGHT_GREEN   0xA
#define CONSOLE_COLOR_LIGHT_CYAN    0xB
#define CONSOLE_COLOR_LIGHT_RED     0xC
#define CONSOLE_COLOR_LIGHT_MAGENTA 0xD
#define CONSOLE_COLOR_YELLOW        0xE
#define CONSOLE_COLOR_WHITE         0xF

// Text attributes
#define CONSOLE_ATTR(fg, bg) ((bg << 4) | (fg & 0x0F))

// Default console attributes
#define CONSOLE_DEFAULT_FG CONSOLE_COLOR_LIGHT_GRAY
#define CONSOLE_DEFAULT_BG CONSOLE_COLOR_BLACK
#define CONSOLE_DEFAULT_ATTR CONSOLE_ATTR(CONSOLE_DEFAULT_FG, CONSOLE_DEFAULT_BG)

// Console structure
typedef struct {
    uint8_t* buffer;
    uint32_t width;
    uint32_t height;
    uint32_t x;
    uint32_t y;
    uint8_t attr;
} console_t;

// Function declarations
void console_init(void);
void console_putc(char c);
void console_puts(const char* str);
void console_clear(void);
void console_set_attr(uint8_t attr);
void console_set_color(uint8_t fg, uint8_t bg);
void console_scroll(void);
void console_put_hex(uint32_t n);
void console_put_dec(uint32_t n);

#endif // _KERNEL_CONSOLE_H
