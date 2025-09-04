#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/syscall.h>

// Standart dosya tanımlayıcılar
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// Karakter yazma
int putchar(int c) {
    char ch = (char)c;
    write(STDOUT_FILENO, &ch, 1);
    return c;
}

// String yazma
int puts(const char* str) {
    size_t len = strlen(str);
    write(STDOUT_FILENO, str, len);
    return putchar('\n');
}

// Formatlı çıktı (basit versiyon)
int vprintf(const char* format, va_list args); // forward declaration
int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}

// Değişken argümanlı formatlı çıktı (basit versiyon)
int vprintf(const char* format, va_list args) {
    char buffer[256];
    int pos = 0;
    
    for (size_t i = 0; format[i] && pos < 255; i++) {
        if (format[i] == '%') {
            i++;
            switch (format[i]) {
                case 'd':
                case 'i': {
                    int num = va_arg(args, int);
                    if (num < 0) {
                        buffer[pos++] = '-';
                        num = -num;
                    }
                    
                    char num_str[16];
                    int num_len = 0;
                    do {
                        num_str[num_len++] = '0' + (num % 10);
                        num /= 10;
                    } while (num > 0);
                    
                    for (int j = num_len - 1; j >= 0; j--) {
                        buffer[pos++] = num_str[j];
                    }
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    while (*str && pos < 255) {
                        buffer[pos++] = *str++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    buffer[pos++] = c;
                    break;
                }
                case 'x':
                case 'X': {
                    unsigned int num = va_arg(args, unsigned int);
                    const char* hex = "0123456789ABCDEF";
                    
                    buffer[pos++] = '0';
                    buffer[pos++] = 'x';
                    
                    int shift = 28;
                    while (shift >= 0) {
                        int nibble = (num >> shift) & 0xF;
                        if (nibble != 0 || shift == 0 || (shift < 28 && pos > 2)) {
                            buffer[pos++] = hex[nibble];
                        }
                        shift -= 4;
                    }
                    break;
                }
                case '%':
                    buffer[pos++] = '%';
                    break;
                default:
                    buffer[pos++] = '%';
                    buffer[pos++] = format[i];
                    break;
            }
        } else {
            buffer[pos++] = format[i];
        }
    }
    
    buffer[pos] = '\0';
    write(STDOUT_FILENO, buffer, pos);
    return pos;
}

// Some toolchains (FORTIFY) emit __printf_chk; provide a compatibility shim.
int __printf_chk(int flag, const char* format, ...) {
    (void)flag;
    va_list args;
    va_start(args, format);
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}

// Minimal snprintf/vsnprintf supporting %s and %d/%x/%c and %%
int vsnprintf(char* buf, size_t size, const char* format, va_list args) {
    if (!buf || size == 0) return 0;
    size_t pos = 0;
    for (size_t i = 0; format[i] && pos + 1 < size; i++) {
        if (format[i] != '%') { buf[pos++] = format[i]; continue; }
        i++;
        if (!format[i]) break;
        switch (format[i]) {
            case 's': {
                const char* s = va_arg(args, const char*);
                if (!s) s = "(null)";
                while (*s && pos + 1 < size) buf[pos++] = *s++;
                break; }
            case 'd': {
                int v = va_arg(args, int); char tmp[32]; int t=0,neg=0; if(v<0){neg=1; v=-v;}
                do { tmp[t++] = '0' + (v%10); v/=10; } while(v&&t<(int)sizeof(tmp));
                if (neg && pos+1<size) buf[pos++]='-';
                while(t && pos+1<size) buf[pos++]=tmp[--t];
                break; }
            case 'x': {
                unsigned int v = va_arg(args,unsigned int); const char* he="0123456789abcdef"; char tmp[32]; int t=0; do { tmp[t++]=he[v&0xF]; v>>=4; } while(v&&t<(int)sizeof(tmp)); while(t && pos+1<size) buf[pos++]=tmp[--t]; break; }
            case 'c': { int ch = va_arg(args,int); buf[pos++] = (char)ch; break; }
            case '%': buf[pos++] = '%'; break;
            default: buf[pos++] = '%'; if (pos + 1 < size) buf[pos++] = format[i]; break;
        }
    }
    buf[pos] = '\0';
    return (int)pos;
}

int snprintf(char* buf, size_t size, const char* format, ...) {
    va_list ap; va_start(ap, format); int r = vsnprintf(buf, size, format, ap); va_end(ap); return r;
}

// Minimal getchar/fgets reading from stdin via SYS_READ
int getchar(void) {
    char ch; long r = SYSCALL3(SYS_READ, 0, &ch, 1); if (r <= 0) return EOF; return (unsigned char)ch;
}

char* fgets(char* s, int size, FILE* stream) {
    (void)stream; if (!s || size <= 1) return NULL; int i=0; while (i < size-1) { int c = getchar(); if (c == EOF) break; s[i++] = (char)c; if (c == '\n') break; } s[i] = '\0'; return (i==0)?NULL:s;
}

#if 0
// File descriptor wrappers moved to unistd.c; keep stubs disabled here.
ssize_t read(int fd, void* buf, size_t count) { return (ssize_t)SYSCALL3(SYS_READ, fd, buf, count); }
ssize_t write(int fd, const void* buf, size_t count) { return (ssize_t)SYSCALL3(SYS_WRITE, fd, buf, count); }
int open(const char* pathname, int flags, ...) { return (int)SYSCALL2(SYS_OPEN, pathname, flags); }
int close(int fd) { return (int)SYSCALL1(SYS_CLOSE, fd); }
#endif

#if 0
// Process/syscall wrappers now live in unistd.c
void _exit(int status) { SYSCALL1(SYS_EXIT, status); while (1) {} }
pid_t getpid(void) { return (pid_t)SYSCALL0(SYS_GETPID); }
pid_t getppid(void) { return (pid_t)SYSCALL0(SYS_GETPPID); }
void* sbrk(intptr_t increment) { return (void*)SYSCALL1(SYS_SBRK, increment); }
pid_t fork(void) { return (pid_t)SYSCALL0(SYS_FORK); }
int execve(const char* path, char* const argv[], char* const envp[]) { return (int)SYSCALL3(SYS_EXECVE, path, argv, envp); }
pid_t waitpid(pid_t pid, int* status, int options) { return (pid_t)SYSCALL3(SYS_WAITPID, pid, status, options); }
#endif
