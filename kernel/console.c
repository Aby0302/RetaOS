#include "../include/kernel/console.h"
#include <stdint.h>
#include <stddef.h>

// Use VGA text mode buffer address directly for CLI
#define VGA_TEXT_ADDR 0xB8000

// Console dimensions
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25

// Current console state
uint16_t* vga_buffer;
uint32_t console_x = 0;
uint32_t console_y = 0;
uint8_t console_attr = CONSOLE_DEFAULT_ATTR;

// Initialize the console
void console_init(void) {
    // Use VGA text buffer directly
    vga_buffer = (uint16_t*)VGA_TEXT_ADDR;
    console_clear();
}



// Clear the console
void console_clear(void) {
    for (size_t y = 0; y < CONSOLE_HEIGHT; y++) {
        for (size_t x = 0; x < CONSOLE_WIDTH; x++) {
            const size_t index = y * CONSOLE_WIDTH + x;
            vga_buffer[index] = (console_attr << 8) | ' ';
        }
    }
    console_x = 0;
    console_y = 0;
}

// Scroll the console up by one line
void console_scroll(void) {
    // Move all lines up by one
    for (size_t y = 1; y < CONSOLE_HEIGHT; y++) {
        for (size_t x = 0; x < CONSOLE_WIDTH; x++) {
            const size_t from_index = y * CONSOLE_WIDTH + x;
            const size_t to_index = (y - 1) * CONSOLE_WIDTH + x;
            vga_buffer[to_index] = vga_buffer[from_index];
        }
    }
    
    // Clear the last line
    const size_t last_line_start = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
    for (size_t x = 0; x < CONSOLE_WIDTH; x++) {
        vga_buffer[last_line_start + x] = (console_attr << 8) | ' ';
    }
    
    if (console_y > 0) {
        console_y--;
    }
}

// Put a character at the current cursor position
void console_putc(char c) {
    if (c == '\n') {
        console_x = 0;
        console_y++;
    } else if (c == '\r') {
        console_x = 0;
    } else if (c == '\t') {
        console_x = (console_x + 8) & ~7;
    } else if (c >= ' ') {
        const size_t index = console_y * CONSOLE_WIDTH + console_x;
        vga_buffer[index] = (console_attr << 8) | c;
        console_x++;
    }
    
    // Handle line wrapping and scrolling
    if (console_x >= CONSOLE_WIDTH) {
        console_x = 0;
        console_y++;
    }
    
    if (console_y >= CONSOLE_HEIGHT) {
        console_scroll();
    }
}

// Output a null-terminated string
void console_puts(const char* str) {
    while (*str) {
        console_putc(*str++);
    }
}

// Set the console text attribute
void console_set_attr(uint8_t attr) {
    console_attr = attr;
}

// Set the console foreground and background colors
void console_set_color(uint8_t fg, uint8_t bg) {
    console_attr = (bg << 4) | (fg & 0x0F);
}

// Get the current cursor position
void console_get_cursor(uint32_t* x, uint32_t* y) {
    if (x) *x = console_x;
    if (y) *y = console_y;
}

// Set the cursor position
void console_set_cursor(uint32_t x, uint32_t y) {
    if (x < CONSOLE_WIDTH) console_x = x;
    if (y < CONSOLE_HEIGHT) console_y = y;
}

// Print a 32-bit value in hexadecimal (8 digits, uppercase)
void console_put_hex(uint32_t n) {
    for (int i = 7; i >= 0; --i) {
        uint8_t nibble = (n >> (i * 4)) & 0xF;
        char ch = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
        console_putc(ch);
    }
}

// Print an unsigned integer in decimal
void console_put_dec(uint32_t n) {
    char buf[11];
    int i = 0;
    if (n == 0) { console_putc('0'); return; }
    while (n && i < (int)sizeof(buf)) { buf[i++] = '0' + (n % 10); n /= 10; }
    while (i > 0) console_putc(buf[--i]);
}
