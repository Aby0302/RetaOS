#include <unistd.h>
#include <string.h>
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
    return putchar('
');
}

// Formatlı çıktı (basit versiyon)
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

// Dosyadan okuma
ssize_t read(int fd, void* buf, size_t count) {
    return (ssize_t)SYSCALL3(SYS_READ, fd, buf, count);
}

// Dosyaya yazma
ssize_t write(int fd, const void* buf, size_t count) {
    return (ssize_t)SYSCALL3(SYS_WRITE, fd, buf, count);
}

// Dosya açma
int open(const char* pathname, int flags) {
    return (int)SYSCALL2(SYS_OPEN, pathname, flags);
}

// Dosya kapatma
int close(int fd) {
    return (int)SYSCALL1(SYS_CLOSE, fd);
}

// Programı sonlandır
void _exit(int status) {
    SYSCALL1(SYS_EXIT, status);
    while (1) {} // Asla buraya düşmemeli
}

// Çalışan işlemin PID'sini döndür
pid_t getpid(void) {
    return (pid_t)SYSCALL0(SYS_GETPID);
}

// Ebeveyn işlemin PID'sini döndür
pid_t getppid(void) {
    return (pid_t)SYSCALL0(SYS_GETPPID);
}

// Program kesme noktasını değiştir (basit bellek yönetimi)
void* sbrk(intptr_t increment) {
    return (void*)SYSCALL1(SYS_SBRK, increment);
}

// Yeni bir işlem oluştur
pid_t fork(void) {
    return (pid_t)SYSCALL0(SYS_FORK);
}

// Program yükle ve çalıştır
int execve(const char* path, char* const argv[], char* const envp[]) {
    return (int)SYSCALL3(SYS_EXECVE, path, argv, envp);
}

// Çocuk işlemin bitmesini bekle
pid_t waitpid(pid_t pid, int* status, int options) {
    return (pid_t)SYSCALL3(SYS_WAITPID, pid, status, options);
}
