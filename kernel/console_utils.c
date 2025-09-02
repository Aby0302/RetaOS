#include "../include/kernel/console.h"
#include "../include/drivers/keyboard.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

// Implementation of vprintf for the console
void console_vprintf(const char* format, va_list args) {
    char buffer[32];
    char* str;
    int num;
    unsigned int unum;
    
    while (*format) {
        if (*format == '%') {
            format++;
            
            // Check for long modifier
            
            switch (*format) {
                case 'd':
                case 'i':
                    num = va_arg(args, int);
                    if (num < 0) {
                        console_putc('-');
                        num = -num;
                    }
                    // Simple itoa implementation
                    int i = 0;
                    do {
                        buffer[i++] = '0' + (num % 10);
                        num /= 10;
                    } while (num > 0 && i < 30);
                    while (i > 0) {
                        console_putc(buffer[--i]);
                    }
                    break;
                    
                case 'u':
                    unum = va_arg(args, unsigned int);
                    i = 0;
                    do {
                        buffer[i++] = '0' + (unum % 10);
                        unum /= 10;
                    } while (unum > 0 && i < 30);
                    while (i > 0) {
                        console_putc(buffer[--i]);
                    }
                    break;
                    
                case 'x':
                case 'X':
                    unum = va_arg(args, unsigned int);
                    i = 0;
                    do {
                        int digit = unum % 16;
                        buffer[i++] = (digit < 10) ? ('0' + digit) : ('a' + (digit - 10));
                        unum /= 16;
                    } while (unum > 0 && i < 30);
                    while (i < 8) {
                        buffer[i++] = '0';
                    }
                    i = (i > 8) ? 8 : i;
                    while (i > 0) {
                        console_putc(buffer[--i]);
                    }
                    break;
                    
                case 'c':
                    console_putc((char)va_arg(args, int));
                    break;
                    
                case 's':
                    str = va_arg(args, char*);
                    while (*str) {
                        console_putc(*str++);
                    }
                    break;
                    
                case '%':
                    console_putc('%');
                    break;
                    
                default:
                    // Unknown format specifier, just print it
                    console_putc('%');
                    console_putc(*format);
                    break;
            }
        } else {
            console_putc(*format);
        }
        format++;
    }
}

// Wrapper for console_vprintf
void console_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    console_vprintf(format, args);
    va_end(args);
}
