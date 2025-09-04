#include "../include/drivers/serial.h"
#include "../include/kernel/sched.h"
#include "../include/kernel/block.h"
#include "../include/kernel/vfs.h"
#include "../include/arch/x86/acpi.h"
#include <kernel/thread.h>
#include <kernel/process.h>
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
    writes("\nRetaOS Kernel Shell - Type 'help' for commands\n\n");
    serial_write("RetaOS Kernel Shell - Type 'help' for commands\r\n\r\n");
    serial_write("[DEBUG] Shell welcome message written\r\n");
    
    for(;;) {
        writes("RetaOSKernel> ");
        serial_write("RetaOSKernel> ");
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
            // Show scheduler + thread summary
            int cnt = scheduler_task_count();
            int curi = scheduler_current_index();
            int q = scheduler_get_quantum();
            int pre = scheduler_get_preempt();
            kprintf("tasks=%d current_index=%d quantum=%d preempt=%d\n", cnt, curi, q, pre);

            // Enumerate threads
            extern void writes(const char*);
            writes("TID   PID   STATE      SLICE\n");
            writes("------------------------------\n");
            thread_t* t = thread_list_head();
            while (t) {
                const char* st = "?";
                switch (t->state) {
                    case THREAD_NEW: st = "NEW"; break;
                    case THREAD_READY: st = "READY"; break;
                    case THREAD_RUNNING: st = "RUNNING"; break;
                    case THREAD_BLOCKED: st = "BLOCKED"; break;
                    case THREAD_TERMINATED: st = "TERM"; break;
                }
                int pid = t->process ? (int)t->process->pid : 0;
                kprintf("%-5d %-5d %-9s %5d\n", (int)t->tid, pid, st, t->time_slice);
                t = t->next;
            }
        } else {
            kprintf("Unknown command: %s\n", line);
        }
    }
}
