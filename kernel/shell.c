#include "../include/drivers/serial.h"
#include "../include/kernel/sched.h"
#include "../include/kernel/block.h"
#include "../include/kernel/vfs.h"
#include "../include/arch/x86/acpi.h"
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

#define LINE_MAX 256

// Forward declarations
extern void kprintf(const char* fmt, ...);
extern void writes(const char* s);
extern void put(char c);
extern int read_line(char* out, int maxlen);
extern void terminal_clear_screen(void);

// Simple string comparison
static int kstrcmp(const char* a, const char* b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a - *b;
}

// Simple shell thread function
void shell_thread(void* arg) {
    (void)arg;
    char line[LINE_MAX];
    
    // Debug: Add serial output to trace shell thread execution
    serial_write("[DEBUG] Shell thread started!\r\n");
    writes("\nRetaOS Shell - Type 'help' for commands\n\n");
    serial_write("[DEBUG] Shell welcome message written\r\n");
    
    for(;;) {
        writes("RetaOS> ");
        int n = read_line(line, LINE_MAX);
        if (n <= 0) continue;
        
        // Simple command parsing
        if (kstrcmp(line, "help") == 0) {
            writes("Available commands:\n");
            writes("  help     - show this help\n");
            writes("  clear    - clear screen\n");
            writes("  version  - show kernel version\n");
            writes("  ps       - show process status\n");
        } else if (kstrcmp(line, "clear") == 0) {
            terminal_clear_screen();
        } else if (kstrcmp(line, "version") == 0) {
            kprintf("RetaOS Kernel v1.0\n");
        } else if (kstrcmp(line, "ps") == 0) {
            kprintf("Process status: TODO\n");
        } else {
            kprintf("Unknown command: %s\n", line);
        }
    }
}