#ifndef _KERNEL_DEBUG_H
#define _KERNEL_DEBUG_H

#include <stdint.h>
#include <stddef.h>

// Hata ayıklama seviyeleri
typedef enum {
    DEBUG_LEVEL_TRACE = 0,  // Detaylı izleme
    DEBUG_LEVEL_DEBUG,      // Hata ayıklama bilgileri
    DEBUG_LEVEL_INFO,       // Bilgilendirici mesajlar
    DEBUG_LEVEL_WARN,       // Uyarı mesajları
    DEBUG_LEVEL_ERROR,      // Hata mesajları
    DEBUG_LEVEL_FATAL,      // Kritik hatalar
    DEBUG_LEVEL_NONE        // Hiçbir şey gösterme
} debug_level_t;

// Hata ayıklama fonksiyonları
void debug_set_level(debug_level_t level);
debug_level_t debug_get_level(void);

// Hata ayıklama çıktı fonksiyonları
void debug_print(debug_level_t level, const char* file, int line, const char* func, const char* fmt, ...);
void debug_dump_memory(const void* addr, size_t size, const char* desc);
void debug_print_registers(void);
void debug_print_stack_trace(unsigned int max_frames);
void debug_print_process_list(void);
void debug_print_memory_map(void);

// Hata ayıklama makroları
#define DEBUG_TRACE(fmt, ...) \
    debug_print(DEBUG_LEVEL_TRACE, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define DEBUG_DEBUG(fmt, ...) \
    debug_print(DEBUG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define DEBUG_INFO(fmt, ...) \
    debug_print(DEBUG_LEVEL_INFO, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define DEBUG_WARN(fmt, ...) \
    debug_print(DEBUG_LEVEL_WARN, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define DEBUG_ERROR(fmt, ...) \
    debug_print(DEBUG_LEVEL_ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define DEBUG_FATAL(fmt, ...) \
    debug_print(DEBUG_LEVEL_FATAL, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

// Assert makrosu
#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            DEBUG_FATAL("Assertion failed: %s, file %s, line %d\n", \
                      #condition, __FILE__, __LINE__); \
            for(;;) asm volatile("cli; hlt"); \
        } \
    } while(0)

// Hata ayıklama komutları
void debug_command(const char* cmd);

#endif // _KERNEL_DEBUG_H
