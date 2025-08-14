// Minimal kernel for RetaOS
#include "include/drivers/serial.h"
#include "include/arch/x86/gdt.h"
#include "include/arch/x86/idt.h"
#include "include/arch/x86/isr.h"
#include "include/drivers/keyboard.h"
#include <stdarg.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long size_t;

// VGA colors
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_WHITE = 15
};

// VGA entry functions
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

// String length function
static size_t strlen_(const char* s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

// Terminal variables
static const size_t W = 80, H = 25;
static size_t r, c;
static uint8_t col;
static uint16_t* buf;

// Forward declaration (defined later)
static void term_scroll(void);
static void set_color(enum vga_color fg, enum vga_color bg);

// Terminal initialization
static void term_init(void) {
    r = 0;
    c = 0;
    col = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    buf = (uint16_t*) 0xB8000;
    
    for (size_t y = 0; y < H; y++) {
        for (size_t x = 0; x < W; x++) {
            buf[y * W + x] = vga_entry(' ', col);
        }
    }
}

// Put character at specific position
static void putat(char ch, uint8_t color, size_t x, size_t y) {
    buf[y * W + x] = vga_entry(ch, color);
}

// Put character at current position
static void put(char ch) {
    if (ch == '\n') {
        c = 0;
        r++;
        term_scroll();
        return;
    }
    putat(ch, col, c, r);
    if (++c == W) {
        c = 0;
        r++;
        term_scroll();
    }
}

// Write string with length
static void write(const char* d, size_t n) {
    for (size_t i = 0; i < n; i++) {
        put(d[i]);
    }
}

// Write string
static void writes(const char* d) {
    write(d, strlen_(d));
}

// Integer printing helpers
static void itoa_unsigned(unsigned int val, unsigned int base, char* buf_out) {
    static const char digits[] = "0123456789abcdef";
    char tmp[32];
    int i = 0;
    if (val == 0) { buf_out[0] = '0'; buf_out[1] = '\0'; return; }
    while (val && i < (int)sizeof(tmp)) { tmp[i++] = digits[val % base]; val /= base; }
    int j = 0; while (i > 0) buf_out[j++] = tmp[--i]; buf_out[j] = '\0';
}

static void itoa_signed(int val, char* buf_out) {
    if (val < 0) { *buf_out++ = '-'; itoa_unsigned((unsigned int)(-val), 10, buf_out); }
    else { itoa_unsigned((unsigned int)val, 10, buf_out); }
}

// Minimal printf-like formatter: supports %s %d %x %c
static void kvprintf(const char* fmt, va_list ap) {
    for (const char* p = fmt; *p; ++p) {
        if (*p != '%') { put(*p); continue; }
        ++p;
        if (!*p) break;
        char numbuf[34];
        switch (*p) {
            case 's': {
                const char* s = va_arg(ap, const char*);
                if (!s) s = "(null)";
                writes(s);
            } break;
            case 'd': {
                int v = va_arg(ap, int);
                char tmp[40];
                itoa_signed(v, tmp);
                writes(tmp);
            } break;
            case 'x': {
                unsigned int v = va_arg(ap, unsigned int);
                itoa_unsigned(v, 16, numbuf);
                writes(numbuf);
            } break;
            case 'c': {
                int ch = va_arg(ap, int);
                put((char)ch);
            } break;
            case '%': put('%'); break;
            default: put('%'); put(*p); break;
        }
    }
}

static void kprintf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    kvprintf(fmt, ap);
    va_end(ap);
}

static void kprintf_color(enum vga_color fg, enum vga_color bg, const char* fmt, ...) {
    uint8_t old = col;
    set_color(fg, bg);
    va_list ap; va_start(ap, fmt);
    kvprintf(fmt, ap);
    va_end(ap);
    col = old;
}

// Forward declarations for memory init (implemented in new modules)
void mb_parse_and_init(uint32_t multiboot_magic, uint32_t multiboot_info_addr);
void paging_init(void);
void kheap_init(void);
void* kmalloc(unsigned long size);

// Terminal enhancements
static void term_scroll(void){
    if (r < H) return;
    for (size_t y = 1; y < H; y++){
        for (size_t x = 0; x < W; x++){
            buf[(y-1)*W + x] = buf[y*W + x];
        }
    }
    for (size_t x = 0; x < W; x++) buf[(H-1)*W + x] = vga_entry(' ', col);
    r = H - 1;
}
static void set_color(enum vga_color fg, enum vga_color bg){ col = vga_entry_color(fg, bg); }
// removed unused puts_ wrapper

// Minimal strcmp replacement (since we are freestanding and -fno-builtin)
static int kstrcmp(const char* a, const char* b){
    while (*a && (*a == *b)) { a++; b++; }
    return (int)((unsigned char)*a) - (int)((unsigned char)*b);
}

// Redraw helper for the in-line editor (prints current line and positions cursor)
static void redraw_line(const char* line, int len, int cur, int input_row, int input_col_start){
    for (int i = 0; i < len && (input_col_start + i) < (int)W; i++)
        putat(line[i], col, input_col_start + i, input_row);
    for (int i = len; (input_col_start + i) < (int)W; i++)
        putat(' ', col, input_col_start + i, input_row);
    r = input_row;
    c = input_col_start + cur;
    if (c >= W) { c = W - 1; }
}

// Clear entire screen and reset cursor
static void terminal_clear_screen(void){
    for (size_t rr = 0; rr < H; rr++){
        for (size_t cc = 0; cc < W; cc++){
            putat(' ', col, (int)cc, (int)rr);
        }
    }
    r = 0; c = 0;
}

// Handle backspace on the VGA terminal
// Backspace helper no longer used (line editor handles erase); keep for reference if needed
// static void term_backspace(void){
//     if (c > 0){
//         c--;
//         putat(' ', col, c, r);
//     } else if (r > 0){
//         r--;
//         c = W;
//         if (c > 0) { c--; }
//         putat(' ', col, c, r);
//     }
// }

// Main kernel function
void kernel_main(uint32_t mb_magic, uint32_t mb_info_addr) {
    // Early serial for debugging
    serial_init();
    serial_write("[RetaOS] Serial initialized.\n");

    // VGA init
    term_init();
    set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    writes("Welcome to RetaOS!\n");

    // Setup GDT and IDT
    gdt_init();
    serial_write("[RetaOS] GDT loaded.\n");

    idt_init();
    serial_write("[RetaOS] IDT loaded.\n");

    // Multiboot memory map and physical memory manager init
    mb_parse_and_init(mb_magic, mb_info_addr);

    // Enable basic paging
    paging_init();

    // Initialize simple kernel heap
    kheap_init();
    // Demo: allocate a few blocks
    void* a = kmalloc(32);
    void* b = kmalloc(128);
    serial_write("[Heap] kmalloc a="); serial_write_hex((unsigned int)(unsigned long)a); serial_write(" b="); serial_write_hex((unsigned int)(unsigned long)b); serial_write("\n");
    // Demonstrate kprintf usage to avoid unused warnings and show on VGA
    kprintf("[Heap] a=%x b=%x\n", (unsigned int)(unsigned long)a, (unsigned int)(unsigned long)b);

    // Keyboard init
    keyboard_init();

    // Install basic IRQs (timer+keyboard), then enable interrupts
    irq_init_basic();
    interrupts_enable();
    serial_write("[RetaOS] IRQs enabled.\n");

    kprintf_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK, "Type on keyboard; in-line editing enabled.\n");

    // Simple in-line editor state + command history
    static char line[256]; int len = 0; int cur = 0;
    static char history[16][256]; int hist_count = 0; int hist_browse = -1; // -1 means editing current line
    int input_row = r; // current row after the header line
    int input_col_start = c; // starting column for the input line

    // Redraw the current input line and place the cursor is done via redraw_line()

    // Process keys forever
    for (;;) {
        int k = keyboard_getkey_nonblock();
        if (k != -1) {
            if (k == 8) { // Backspace
                if (cur > 0) {
                    // Shift left from cur-1
                    for (int i = cur - 1; i < len - 1; i++) line[i] = line[i + 1];
                    len--; cur--; line[len] = 0;
                }
                redraw_line(line, len, cur, input_row, input_col_start);
            } else if (k == '\n') {
                // Commit line: print newline, optionally echo line back
                put('\n');
                // Handle basic commands: help, clear, echo
                if (len > 0) {
                    // Save to history if different from last entry
                    if (hist_count == 0 || kstrcmp(history[(hist_count-1) % 16], line) != 0){
                        int idx = hist_count % 16;
                        int i = 0; while (i < 255 && line[i]) { history[idx][i] = line[i]; i++; }
                        history[idx][i] = '\0';
                        hist_count++;
                    }
                    hist_browse = -1;

                    // Parse command
                    // Find first space
                    int sp = 0; while (sp < len && line[sp] != ' ') sp++;
                    // Command
                    char cmd[16]; int ci = 0; while (ci < 15 && ci < sp) { cmd[ci] = line[ci]; ci++; } cmd[ci] = 0;
                    // Arg start
                    int ai = sp; while (ai < len && line[ai] == ' ') ai++;
                    if (cmd[0] == 0) {
                        // do nothing
                    } else if (kstrcmp(cmd, "help") == 0) {
                        kprintf("Commands: help, clear, echo\n");
                    } else if (kstrcmp(cmd, "clear") == 0) {
                        terminal_clear_screen();
                        kprintf("Type on keyboard; in-line editing enabled.\n");
                        input_row = r; input_col_start = c;
                    } else if (kstrcmp(cmd, "echo") == 0) {
                        if (ai < len) {
                            // print rest of line
                            char tmp = line[len]; // ensure null
                            line[len] = 0;
                            kprintf("%s\n", &line[ai]);
                            line[len] = tmp;
                        } else {
                            kprintf("\n");
                        }
                    } else {
                        kprintf("command not found: %s\n", cmd);
                    }
                }
                // Reset editor state on new prompt line
                len = 0; cur = 0; line[0] = 0;
                input_row = r; input_col_start = c;
                redraw_line(line, len, cur, input_row, input_col_start);
            } else if (k >= KBD_KEY_BASE) {
                // Special keys
                if (k == KBD_LEFT) { if (cur > 0) { cur--; } redraw_line(line, len, cur, input_row, input_col_start); }
                else if (k == KBD_RIGHT) { if (cur < len) { cur++; } redraw_line(line, len, cur, input_row, input_col_start); }
                else if (k == KBD_HOME) { cur = 0; redraw_line(line, len, cur, input_row, input_col_start); }
                else if (k == KBD_END) { cur = len; redraw_line(line, len, cur, input_row, input_col_start); }
                else if (k == KBD_DEL) {
                    if (cur < len) {
                        for (int i = cur; i < len - 1; i++) line[i] = line[i + 1];
                        len--; line[len] = 0; redraw_line(line, len, cur, input_row, input_col_start);
                    }
                } else if (k == KBD_UP) {
                    if (hist_count > 0){
                        if (hist_browse < 0) hist_browse = hist_count - 1; else if (hist_browse > 0) hist_browse--;
                        int idx = hist_browse % 16;
                        // load history entry
                        int i = 0; while (i < 255 && history[idx][i]) { line[i] = history[idx][i]; i++; } line[i] = 0;
                        len = i; cur = len;
                        redraw_line(line, len, cur, input_row, input_col_start);
                    }
                } else if (k == KBD_DOWN) {
                    if (hist_count > 0 && hist_browse >= 0){
                        if (hist_browse < hist_count - 1) { hist_browse++; }
                        if (hist_browse >= hist_count - 1){
                            // back to empty current line
                            hist_browse = -1; len = 0; cur = 0; line[0] = 0;
                        } else {
                            int idx = hist_browse % 16;
                            int i = 0; while (i < 255 && history[idx][i]) { line[i] = history[idx][i]; i++; } line[i] = 0;
                            len = i; cur = len;
                        }
                        redraw_line(line, len, cur, input_row, input_col_start);
                    }
                }
            } else if (k >= 32 && k < 127) {
                // Insert printable ASCII at cursor
                if (len < (int)sizeof(line) - 1) {
                    for (int i = len; i > cur; i--) line[i] = line[i - 1];
                    line[cur] = (char)k; len++; cur++; line[len] = 0;
                }
                redraw_line(line, len, cur, input_row, input_col_start);
            } else {
                // Ignore other controls
            }
        }
        __asm__ __volatile__("hlt");
    }
}
