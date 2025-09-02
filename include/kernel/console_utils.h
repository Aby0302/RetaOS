#ifndef _KERNEL_CONSOLE_UTILS_H
#define _KERNEL_CONSOLE_UTILS_H

#include <stdarg.h>

// Function declarations
void console_printf(const char* format, ...);
void console_vprintf(const char* format, va_list args);

#endif // _KERNEL_CONSOLE_UTILS_H
