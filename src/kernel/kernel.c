// Minimal kernel for RetaOS
#include "include/drivers/serial.h"
#include "include/arch/x86/gdt.h"
#include "include/arch/x86/idt.h"
#include "include/arch/x86/isr.h"
#include "include/drivers/keyboard.h"

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
        if (++r == H) r = 0;
        return;
    }
    putat(ch, col, c, r);
    if (++c == W) {
        c = 0;
        if (++r == H) r = 0;
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
static void puts_(const char* s){ writes(s); }

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

    // Keyboard init
    keyboard_init();

    // Install basic IRQs (timer+keyboard), then enable interrupts
    irq_init_basic();
    interrupts_enable();
    serial_write("[RetaOS] IRQs enabled.\n");

    puts_("Type on keyboard; characters are buffered.\n");
    
    // Halt the CPU
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}
