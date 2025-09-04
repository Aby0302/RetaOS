// Minimal kernel for RetaOS
// Removed conflicting multiboot.h - using multiboot2.h instead
#include "include/arch/x86/multiboot2.h"
#include "include/drivers/serial.h"
#include "include/arch/x86/io.h"
//#include "include/kernel/splash.h" // disable splash for CLI
extern void sched_init(void);  // Declaration for the scheduler initialization
#include "include/kernel/scheduler.h"
#include "include/kernel/sched.h"  // For sched_init and sched_start
#include "include/arch/x86/isr.h"  // For idt_init

#include "include/arch/x86/gdt.h"
#include "include/arch/x86/idt.h"
#include "include/arch/x86/isr.h"
#include "include/arch/x86/io.h"
#include "include/drivers/keyboard.h"
#include "include/kernel/sched.h"
#include "include/kernel/block.h"
#include "include/drivers/ata.h"
#include "include/kernel/initrd.h"
#include "include/arch/x86/acpi.h"
#include "include/drivers/keyboard.h"
#include "include/kernel/syscall.h"
#include "include/kernel/syscalls.h"
#include "include/kernel/elf.h"
#include "include/kernel/process.h"
#include "include/kernel/splash.h"
//#include "include/gui/gui.h"       // disable GUI for CLI
//#include "include/kernel/display.h" // disable display framebuffer
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include "include/kernel/console.h"

// Forward declare userspace launcher thread function
static void user_init_launcher(void* arg);

// VGA colors
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
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

// Terminal functions
static void term_init(void) {
    buf = (uint16_t*)0xB8000;
    col = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    r = 0;
    c = 0;
    
    // Clear screen
    for (size_t y = 0; y < H; y++) {
        for (size_t x = 0; x < W; x++) {
            const size_t index = y * W + x;
            buf[index] = vga_entry(' ', col);
        }
    }
    
    // Move cursor to top-left
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(0x00));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)(0x00 >> 8));
}

static void putat(char ch, uint8_t color, size_t x, size_t y) {
    if (x < W && y < H)
        buf[y * W + x] = vga_entry(ch, color);
}

static void put(char ch) {
    if (ch == '\n') {
        c = 0;
        r++;
        if (r >= H) {
            // Simple scroll - just reset to top
            r = 0;
        }
        return;
    }
    // Handle backspace locally (ASCII 8 / '\b')
    if (ch == '\b' || ch == 8) {
        if (c > 0) {
            c--;
        } else if (r > 0) {
            r--;
            c = W - 1;
        } else {
            return; // top-left; nothing to erase
        }
        putat(' ', col, c, r);
        return;
    }
    putat(ch, col, c, r);
    if (++c == W) {
        c = 0;
        r++;
        if (r >= H) {
            r = 0;
        }
    }
}

void writes(const char* d) {
    for (size_t i = 0; d[i]; i++) {
        put(d[i]);
    }
}

// Simple printf implementation
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

void kprintf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    kvprintf(fmt, ap);
    va_end(ap);
}

// Kernel thread that mounts initrd from ATA disk and switches to userspace init
static void user_init_launcher(void* arg) {
    (void)arg;
    extern void ata_init(void);
    extern int initrd_mount_from_block(const char* dev, uint32_t lba_start, uint32_t max_sectors, uint32_t max_bytes_cap);
    extern int elf_exec(const char* filename);

    serial_write("[userinit] Initializing ATA...\r\n");
    ata_init();
    serial_write("[userinit] Mounting initrd from hda (LBA1)...\r\n");
    // Read up to 256KB from LBA1 as initrd (ustar)
    initrd_mount_from_block("hda", 1, 512, 512*1024);

    serial_write("[userinit] Trying elf_exec /bin/init.elf\r\n");
    if (elf_exec("/bin/init.elf") == 0)
        return; // no return on success; elf_exec switches to usermode
    serial_write("[userinit] Trying elf_exec /bin/sh\r\n");
    if (elf_exec("/bin/sh") == 0)
        return;
    #include "include/kernel/bsod.h"
    kernel_bsod("Kullanıcı alanı başlatılamadı!\n\ninit.elf ve sh çalıştırılamadı.\n\nMuhtemel nedenler:\n- initrd mount hatası\n- ELF yükleme/paging hatası\n- Kullanıcı programı bozuk\n\nLütfen logları kontrol edin.");
}

// Terminal functions for shell
void terminal_clear_screen(void) {
    for (size_t rr = 0; rr < H; rr++) {
        for (size_t cc = 0; cc < W; cc++) {
            putat(' ', col, cc, rr);
        }
    }
    r = 0; c = 0;
}

// Simple line reading function
int read_line(char* out, int maxlen) {
    int i = 0;
    char c;
    
    if (!out || maxlen <= 0) return -1;
    
    while (i < maxlen - 1) {
        // Wait for a key from keyboard or serial
        for (;;) {
            int kc = keyboard_getchar_nonblock();
            if (kc >= 0) { c = (char)kc; break; }
            extern int serial_getchar_nonblock(void);
            int sc = serial_getchar_nonblock();
            if (sc >= 0) { c = (char)sc; if (c == '\r') c = '\n'; break; }
            extern void thread_yield(void); thread_yield();
        }
        
        // Handle backspace
        if (c == 8) {
            if (i > 0) {
                i--;
                // Move cursor back, print space, move cursor back again
                put(8);   // Backspace
                put(' '); // Space over the character
                put(8);   // Backspace again
            }
            continue;
        }
        
        // Handle enter key
        if (c == '\n') {
            put(c); // Echo newline
            out[i] = '\0';   // Null terminate
            return i;
        }
        
        // Handle regular characters
        if (c >= ' ' && c <= '~') {
            out[i++] = c;
            put(c); // Echo on VGA
            extern void serial_write(const char*);
            char s[2] = { c, '\0' }; serial_write(s); // Echo on serial
        }
    }
    
    // Buffer full, null terminate and return
    out[i] = '\0';
    return i;
}

// Kernel symbols for memory layout
extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

// Simple multiboot2 parsing
static void parse_multiboot(uint32_t mb_magic, uint32_t mb_info_addr) {
    if (mb_magic != 0x2BADB002 && mb_magic != 0x1BADB002) {
        while (1) __asm__ __volatile__ ("hlt");
    }
    
    uint32_t* mb_info = (uint32_t*)mb_info_addr;
    if (mb_info[0] < 8) {
        while (1) __asm__ __volatile__ ("hlt");
    }
}

// Kernel main function
void _kernel_main(uint32_t mb_magic, uint32_t mb_info_addr) {
    // EARLY DEBUG: Output to serial and VGA to confirm kernel entry
    serial_init();
    serial_write("[DEBUG] Entered kernel_main!\r\n");
    uint16_t* vga = (uint16_t*)0xB8000;
    vga[0] = 0x1F00 | 'K';
    vga[1] = 0x1F00 | 'E';
    vga[2] = 0x1F00 | 'R';
    vga[3] = 0x1F00 | 'N';
    vga[4] = 0x1F00 | 'E';
    vga[5] = 0x1F00 | 'L';
    vga[6] = 0x1F00 | '!';
    vga[7] = 0x1F00 | '!';

    // Initialize VGA text console only
    term_init();

    // Log a few messages to screen and serial
    writes("RetaOS booting in CLI mode...\n");
    serial_write("RetaOS booting in CLI mode...\r\n");
    splash_update_progress(5);
    // Initialize GDT and then interrupt handling
    extern void gdt_init(void);
    gdt_init();
    serial_write("[DEBUG] GDT initialized\r\n");
    splash_update_progress(15);
    // Initialize interrupt handling
    idt_init();  // This will also initialize the PIC
    writes("IDT initialized\n");
    serial_write("[DEBUG] Initialize interrupt handling\r\n");
    splash_update_progress(25);
    // Initialize IRQ handlers (timer and keyboard) - this also sets up the timer
    extern void irq_init_basic(void);
    irq_init_basic();
    writes("IRQ initialized\n");
    serial_write("[DEBUG] Initialize IRQ handlers\r\n");
    splash_update_progress(35);
    // Initialize keyboard
    extern void keyboard_init(void);
    keyboard_init();
    serial_write("[DEBUG] Initialize keyboard\r\n");
    splash_update_progress(45);
    // Enable interrupts globally
    extern void interrupts_enable(void);
    interrupts_enable();
    serial_write("[DEBUG] Interrupts enabled globally\r\n");
    splash_update_progress(55);

    // Show Splash Screen
    splash_show();

    // Initialize memory management
    extern void memory_init(uint32_t mb_magic, uint32_t mb_info_addr);
    memory_init(mb_magic, mb_info_addr);

    // Initialize paging
    extern void paging_init(void);
    paging_init();
    serial_write("[DEBUG] Initialize paging\r\n");
    splash_update_progress(65);
    // Initialize process management
    extern void process_init(void);
    process_init();
    serial_write("[DEBUG] Initialize process management\r\n");
    splash_update_progress(75);
    // Initialize threading system
    extern void threading_init(void);
    threading_init();
    serial_write("[DEBUG] Initialize threading system\r\n");
    splash_update_progress(85);
    // Initialize filesystem and (later) mount initrd
    extern void fs_init(void);
    fs_init();
    serial_write("[DEBUG] Initialize filesystem\r\n");
    splash_update_progress(90);
    // Initialize system calls
    extern void syscall_init(void);
    syscall_init();
    serial_write("[DEBUG] Initialize system calls\r\n");
    splash_update_progress(95);
    // Mount root filesystem
    extern void mount_root(void);
    mount_root();
    serial_write("[DEBUG] Mount root filesystem\r\n");
    splash_update_progress(96);
    // Initialize scheduler
    sched_init();
    serial_write("[DEBUG] Initialize scheduler\r\n");
    splash_update_progress(97);
    //writes("Initialization complete. Starting scheduler...\n");
    splash_update_progress(98);
    serial_write("Initialization complete. Starting scheduler...\r\n");
    splash_update_progress(99);
    serial_write("[DEBUG] Creating user init launcher thread\r\n");
    splash_update_progress(100);

    // Show splash completion and wait for ENTER; control will return via
    // kernel_continue_after_splash() when user presses ENTER.
    splash_show_complete();

    while (1) {
        __asm__ __volatile__ ("cli; hlt");
    }
}

// Called by splash.c after user presses ENTER on splash screen
void kernel_continue_after_splash(void) {
    serial_write("[KERNEL] ENTER pressed; starting scheduler and shell...\r\n");
    thread_create((void*(*)(void*))user_init_launcher, NULL);
    scheduler_start();
}
