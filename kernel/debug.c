#include <kernel/debug.h>
#include <kernel/process.h>
#include <kernel/console.h>
#include <kernel/console_utils.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <kernel/task.h>  // For process_list declaration

// Varsayılan hata ayıklama seviyesi
static debug_level_t current_debug_level = DEBUG_LEVEL_INFO;

// Hata ayıklama seviyesini ayarla
void debug_set_level(debug_level_t level) {
    if (level >= DEBUG_LEVEL_TRACE && level <= DEBUG_LEVEL_NONE) {
        current_debug_level = level;
    }
}

// Mevcut hata ayıklama seviyesini al
debug_level_t debug_get_level(void) {
    return current_debug_level;
}

// Hata ayıklama çıktısı için renk kodları
#define COLOR_TRACE "\x1b[36m"  // Cyan
#define COLOR_DEBUG "\x1b[34m"  // Mavi
#define COLOR_INFO  "\x1b[32m"  // Yeşil
#define COLOR_WARN  "\x1b[33m"  // Sarı
#define COLOR_ERROR "\x1b[31m"  // Kırmızı
#define COLOR_FATAL "\x1b[35m"  // Mor
#define COLOR_RESET "\x1b[0m"   // Varsayılan renk

// Seviyeye göre renk döndür
static const char* get_level_color(debug_level_t level) {
    switch (level) {
        case DEBUG_LEVEL_TRACE: return COLOR_TRACE;
        case DEBUG_LEVEL_DEBUG: return COLOR_DEBUG;
        case DEBUG_LEVEL_INFO:  return COLOR_INFO;
        case DEBUG_LEVEL_WARN:  return COLOR_WARN;
        case DEBUG_LEVEL_ERROR: return COLOR_ERROR;
        case DEBUG_LEVEL_FATAL: return COLOR_FATAL;
        default:                return COLOR_RESET;
    }
}

// Seviye için metin etiketi döndür
static const char* get_level_label(debug_level_t level) {
    switch (level) {
        case DEBUG_LEVEL_TRACE: return "TRACE";
        case DEBUG_LEVEL_DEBUG: return "DEBUG";
        case DEBUG_LEVEL_INFO:  return "INFO ";
        case DEBUG_LEVEL_WARN:  return "WARN ";
        case DEBUG_LEVEL_ERROR: return "ERROR";
        case DEBUG_LEVEL_FATAL: return "FATAL";
        default:                return "UNKNW";
    }
}

// Hata ayıklama mesajı yazdır
void debug_print(debug_level_t level, const char* file, int line, const char* func, const char* fmt, ...) {
    if (level < current_debug_level) {
        return;
    }

    // Renk ve seviye etiketini yazdır
    console_printf("%s[%s] %s:%d:%s()%s ", 
                  get_level_color(level), 
                  get_level_label(level), 
                  file, line, func,
                  COLOR_RESET);
    
    // Mesajı yazdır
    va_list args;
    va_start(args, fmt);
    console_vprintf(fmt, args);
    va_end(args);
    
    // Yeni satır ekle
    console_putc('\n');
}

// Bellek içeriğini hex ve ASCII olarak yazdır
void debug_dump_memory(const void* addr, size_t size, const char* desc) {
    if (desc) {
        DEBUG_INFO("Memory Dump (%s):", desc);
    } else {
        DEBUG_INFO("Memory Dump:");
    }
    
    const uint8_t* ptr = (const uint8_t*)addr;
    
    for (size_t i = 0; i < size; i += 16) {
        // Adresi yazdır
        console_printf("0x%08x: ", (uint32_t)(ptr + i));
        
        // Hex değerleri yazdır
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                console_printf("%02x ", ptr[i + j]);
            } else {
                console_printf("   ");
            }
            
            if (j == 7) console_printf(" ");
        }
        
        // ASCII değerleri yazdır
        console_printf(" |");
        for (size_t j = 0; j < 16 && (i + j) < size; j++) {
            uint8_t c = ptr[i + j];
            if (c >= 32 && c <= 126) {
                console_printf("%c", c);
            } else {
                console_printf(".");
            }
        }
        console_printf("|\n");
    }
}

// Kayıt değerlerini yazdır
void debug_print_registers(void) {
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp, esp, eip, eflags;
    uint16_t cs, ds, es, fs, gs, ss;
    
    // Read general purpose registers first
    __asm__ __volatile__ (
        "movl %%eax, %0\n\t"
        "movl %%ebx, %1\n\t"
        "movl %%ecx, %2\n\t"
        "movl %%edx, %3\n\t"
        "movl %%esi, %4\n\t"
        "movl %%edi, %5\n\t"
        : "=m" (eax), "=m" (ebx), "=m" (ecx), 
          "=m" (edx), "=m" (esi), "=m" (edi)
        :
        : "memory"
    );
    
    // Read stack and base pointers
    __asm__ __volatile__ (
        "movl %%ebp, %0\n\t"
        "movl %%esp, %1\n\t"
        : "=m" (ebp), "=m" (esp)
        :
        : "memory"
    );
    
    // Read EIP from the stack
    __asm__ __volatile__ (
        "movl (%%ebp), %0\n\t"
        "movl 4(%%ebp), %1\n\t"
        : "=r" (ebp), "=r" (eip)
        :
        : "memory"
    );
    
    // Read flags
    __asm__ __volatile__ (
        "pushfl\n\t"
        "popl %0\n\t"
        : "=r" (eflags)
        :
        : "memory"
    );
    
    // Read segment registers
    __asm__ __volatile__ (
        "movw %%cs, %0\n\t"
        "movw %%ds, %1\n\t"
        "movw %%es, %2\n\t"
        "movw %%fs, %3\n\t"
        "movw %%gs, %4\n\t"
        "movw %%ss, %5\n\t"
        : "=m" (cs), "=m" (ds), "=m" (es), "=m" (fs), "=m" (gs), "=m" (ss)
        :
        : "memory"
    );
    
    DEBUG_INFO("Register Dump:");
    console_printf("EAX: 0x%08x  EBX: 0x%08x  ECX: 0x%08x  EDX: 0x%08x\n", eax, ebx, ecx, edx);
    console_printf("ESI: 0x%08x  EDI: 0x%08x  EBP: 0x%08x  ESP: 0x%08x\n", esi, edi, ebp, esp);
    console_printf("EIP: 0x%08x  EFLAGS: 0x%08x\n", eip, eflags);
    console_printf("CS: 0x%04x  DS: 0x%04x  ES: 0x%04x  FS: 0x%04x  GS: 0x%04x  SS: 0x%04x\n", 
                  cs, ds, es, fs, gs, ss);
}

// Yığın izini (stack trace) yazdır
void debug_print_stack_trace(unsigned int max_frames) {
    DEBUG_INFO("Stack trace (max %u frames):", max_frames);
    
    uint32_t* ebp;
    __asm__ __volatile__ ("movl %%ebp, %0" : "=r" (ebp));
    
    for (unsigned int frame = 0; frame < max_frames && ebp; frame++) {
        uint32_t eip = ebp[1];
        console_printf("  [%u] 0x%08x\n", frame, eip);
        ebp = (uint32_t*)ebp[0];
        if (!ebp) break;
    }
}

// İşlem listesini yazdır
void debug_print_process_list(void) {
    DEBUG_INFO("Process List:");
    console_printf("PID\tSTATE\n");
    console_printf("----------------\n");
    
    // Get the current process (for now, we'll just show the current process until we have a proper process list)
    process_t* p = process_current();
    if (p) {
        const char* state;
        switch (p->state) {
            case PROC_NEW: state = "NEW"; break;
            case PROC_RUNNING: state = "RUNNING"; break;
            case PROC_READY: state = "READY"; break;
            case PROC_BLOCKED: state = "BLOCKED"; break;
            case PROC_TERMINATED: state = "TERMINATED"; break;
            default: state = "UNKNOWN";
        }
        
        console_printf("%d\t%s\n", p->pid, state);
    }
}

// Bellek haritasını yazdır (basit bir versiyon)
void debug_print_memory_map(void) {
    DEBUG_INFO("Memory Map (simplified):");
    
    // Kernel alanı
    console_printf("0x00000000 - 0x003FFFFF: Kernel Code/Data\n");
    console_printf("0x00400000 - 0xBFFFFFFF: User Space\n");
    console_printf("0xC0000000 - 0xFFFFFFFF: Kernel Stack/Heap\n");
    
    // Daha detaylı bilgi için kernel bellek yöneticisini kullanabilirsiniz
    DEBUG_INFO("Note: This is a simplified memory map. Use 'meminfo' for details.");
}

// Hata ayıklama komutlarını işle
void debug_command(const char* cmd) {
    if (strncmp(cmd, "regs", 4) == 0) {
        debug_print_registers();
    } else if (strncmp(cmd, "ps", 2) == 0) {
        debug_print_process_list();
    } else if (strncmp(cmd, "stack", 5) == 0) {
        debug_print_stack_trace(16);
    } else if (strncmp(cmd, "memmap", 6) == 0) {
        debug_print_memory_map();
    } else if (strncmp(cmd, "loglevel ", 9) == 0) {
        int level = cmd[9] - '0';
        if (level >= DEBUG_LEVEL_TRACE && level < DEBUG_LEVEL_NONE) {
            debug_set_level((debug_level_t)level);
            DEBUG_INFO("Log level set to %d", level);
        } else {
            DEBUG_ERROR("Invalid log level. Use 0-5 (0=trace, 5=none)");
        }
    } else if (strcmp(cmd, "help") == 0) {
        console_printf("Available debug commands:\n");
        console_printf("  regs      - Show CPU registers\n");
        console_printf("  ps        - Show process list\n");
        console_printf("  stack     - Show stack trace\n");
        console_printf("  memmap    - Show memory map\n");
        console_printf("  loglevel N- Set log level (0-5)\n");
        console_printf("  help      - Show this help\n");
    } else {
        DEBUG_ERROR("Unknown debug command: %s (type 'help' for available commands)", cmd);
    }
}
