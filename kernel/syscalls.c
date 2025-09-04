#include <kernel/syscalls.h>
#include <kernel/process.h>
#include <kernel/vfs.h>
#include <memory/heap.h>
#include <kernel/elf.h>
#include <kernel/sched.h>
#include <drivers/serial.h>  // serial_write için
#include <drivers/keyboard.h>
#include <kernel/console.h>
#include <kernel/thread.h>
#include <gui/display.h>
#include <gui/fb.h>
#include <arch/x86/isr.h>    // isr_context yapısı için
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <gui/display.h>
#include <gui/fb.h>

// Prototypes for custom framebuffer syscalls
int32_t sys_fb_getinfo(void* out, size_t size, uint32_t, uint32_t, uint32_t, uint32_t);
int32_t sys_fb_fill(uint32_t rgb, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
int32_t sys_fb_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t rgb, uint32_t);

// Maksimum syscall sayısı
#define MAX_SYSCALLS 256

// Syscall işleyici tablosu
static syscall_handler_t syscall_table[MAX_SYSCALLS] = {0};

// Syscall işleyicisini kaydet
void syscall_register(uint32_t num, syscall_handler_t handler) {
    if (num < MAX_SYSCALLS) {
        syscall_table[num] = handler;
    }
}

// Syscall'i işle
int32_t syscall_dispatch(uint32_t num, uint32_t arg1, uint32_t arg2, 
                        uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6) {
    if (num >= MAX_SYSCALLS || !syscall_table[num]) {
        return -1; // Geçersiz syscall
    }
    return syscall_table[num](arg1, arg2, arg3, arg4, arg5, arg6);
}

// Syscall handler - isr.S'den çağrılır
int32_t syscall_handler(struct isr_context* ctx) {
    uint32_t num = ctx->eax;
    uint32_t a1 = ctx->ebx;
    uint32_t a2 = ctx->ecx;
    uint32_t a3 = ctx->edx;
    uint32_t a4 = ctx->esi;
    uint32_t a5 = ctx->edi;
    uint32_t a6 = ctx->ebp;
    
    return syscall_dispatch(num, a1, a2, a3, a4, a5, a6);
}

// ==================== SİSTEM ÇAĞRILARI ====================

// exit - İşlemi sonlandır
static int32_t sys_exit(int exit_code, uint32_t unused1, uint32_t unused2, 
                       uint32_t unused3, uint32_t unused4, uint32_t unused5) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4; (void)unused5;
    process_exit(exit_code);
    return 0; // Buraya asla ulaşılmaz
}

// fork - Yeni bir işlem oluştur
static int32_t sys_fork(uint32_t unused1, uint32_t unused2, uint32_t unused3, 
                       uint32_t unused4, uint32_t unused5, uint32_t unused6) {
    (void)unused1; (void)unused2; (void)unused3; 
    (void)unused4; (void)unused5; (void)unused6;
    
    process_t* child = process_create();
    if (!child) return -1;
    
    // TODO: Çocuk işlemin bağlamını kopyala (COW vs.)
    
    return child->pid; // Ebeveynde çocuğun PID'sini döndür
}

// read - Dosyadan okuma
static int32_t sys_read(int fd, void* buf, size_t count, 
                       uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1; (void)unused2; (void)unused3;
    
    // Basit TTY: fd==0 ise klavye VEYA seri porttan bloklayarak oku
    if (fd == 0) { // stdin
        if (!buf || count == 0) return 0;
        char* out = (char*)buf;
        size_t n = 0;
        while (n < count) {
            int ch = keyboard_getchar_nonblock();
            if (ch < 0) {
                // Try serial port as fallback (for -nographic/serial mode)
                extern int serial_getchar_nonblock(void);
                ch = serial_getchar_nonblock();
                // Convert CR to LF for compatibility
                if (ch == '\r') ch = '\n';
            }
            if (ch >= 0) {
                out[n++] = (char)ch;
                // Echo to VGA and serial so -nographic users see input
                console_putc((char)ch);
                char s[2] = { (char)ch, '\0' };
                serial_write(s);
                if (ch == '\n') break; // satır sonu okunduysa çık
            } else {
                // başka thread'lere CPU ver
                thread_yield();
            }
        }
        return (int32_t)n;
    }
    
    // Dosyadan okuma
    return vfs_read(fd, buf, count);
}

// write - Dosyaya yazma
static int32_t sys_write(int fd, const void* buf, size_t count, 
                        uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1; (void)unused2; (void)unused3;
    
    if (fd == 1 || fd == 2) { // stdout veya stderr
        // Konsola yaz ve ayrıca seri porta yansıt
        const char* str = (const char*)buf;
        for (size_t i = 0; i < count; i++) {
            char ch = str[i];
            if (ch == '\0') break;
            console_putc(ch);
            char tmp[2] = { ch, '\0' };
            serial_write(tmp);
        }
        return (int32_t)count;
    }
    
    // Dosyaya yaz
    return vfs_write(fd, buf, count);
}

// open - Dosya açma
static int32_t sys_open(const char* filename, int flags, int mode, 
                       uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1; (void)unused2; (void)unused3;
    return vfs_open(filename, flags);
}

// close - Dosya kapatma
static int32_t sys_close(int fd, uint32_t unused1, uint32_t unused2, 
                        uint32_t unused3, uint32_t unused4, uint32_t unused5) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4; (void)unused5;
    return vfs_close(fd);
}

// execve - Program çalıştırma
static int32_t sys_execve(const char* path, char* const argv[], char* const envp[],
                         uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)argv; (void)envp; (void)unused1; (void)unused2; (void)unused3;
    
    process_t* current = process_current();
    if (!current) return -1;
    
    // Yeni programı yükle ve çalıştır
    if (process_exec(current, path) < 0) {
        return -1;
    }
    
    return 0; // Buraya asla ulaşılmaz
}

// getpid - Mevcut işlem kimliğini döndür
static int32_t sys_getpid(uint32_t unused1, uint32_t unused2, uint32_t unused3, 
                         uint32_t unused4, uint32_t unused5, uint32_t unused6) {
    (void)unused1; (void)unused2; (void)unused3; 
    (void)unused4; (void)unused5; (void)unused6;
    
    process_t* current = process_current();
    return current ? current->pid : -1;
}

// getppid - Ebeveyn işlem kimliğini döndür
static int32_t sys_getppid(uint32_t unused1, uint32_t unused2, uint32_t unused3, 
                          uint32_t unused4, uint32_t unused5, uint32_t unused6) {
    (void)unused1; (void)unused2; (void)unused3; 
    (void)unused4; (void)unused5; (void)unused6;
    
    process_t* current = process_current();
    if (!current) return -1;
    
    process_t* parent = process_find(current->parent_pid);
    return parent ? parent->pid : 0; // Ebeveyn yoksa 0 döndür
}

// sbrk - Program kesme noktasını değiştir
static void* sys_sbrk(intptr_t increment, uint32_t unused1, uint32_t unused2, 
                     uint32_t unused3, uint32_t unused4, uint32_t unused5) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4; (void)unused5;
    
    // TODO: Dinamik bellek yönetimi entegrasyonu
    return (void*)-1; // Şimdilik desteklenmiyor
}

// waitpid - Çocuk işlemin bitmesini bekle
static int32_t sys_waitpid(pid_t pid, int* status, int options, 
                          uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)pid; (void)status; (void)options;
    (void)unused1; (void)unused2; (void)unused3;
    
    // TODO: İşlem bekleme mekanizması
    return -1; // Şimdilik desteklenmiyor
}

// Syscall'ları başlat
void syscalls_init(void) {
    // Temel işlem yönetimi
    syscall_register(SYS_EXIT, (syscall_handler_t)sys_exit);
    syscall_register(SYS_FORK, (syscall_handler_t)sys_fork);
    syscall_register(SYS_EXECVE, (syscall_handler_t)sys_execve);
    syscall_register(SYS_WAITPID, (syscall_handler_t)sys_waitpid);
    syscall_register(SYS_GETPID, (syscall_handler_t)sys_getpid);
    syscall_register(SYS_GETPPID, (syscall_handler_t)sys_getppid);
    
    // Dosya işlemleri
    syscall_register(SYS_READ, (syscall_handler_t)sys_read);
    syscall_register(SYS_WRITE, (syscall_handler_t)sys_write);
    syscall_register(SYS_OPEN, (syscall_handler_t)sys_open);
    syscall_register(SYS_CLOSE, (syscall_handler_t)sys_close);
    
    // Bellek yönetimi
    syscall_register(SYS_SBRK, (syscall_handler_t)sys_sbrk);
    
    // Framebuffer helpers
    syscall_register(SYS_FB_GETINFO, (syscall_handler_t)sys_fb_getinfo);
    syscall_register(SYS_FB_FILL,    (syscall_handler_t)sys_fb_fill);
    syscall_register(SYS_FB_RECT,    (syscall_handler_t)sys_fb_rect);

    serial_write("[syscalls] Initialized system calls\n");
}

// === Framebuffer syscall handlers ===
int32_t sys_fb_getinfo(void* out, size_t size, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6) {
    (void)a3; (void)a4; (void)a5; (void)a6;
    if (!out || size < sizeof(fb_info_t)) return -1;
    fb_info_t* info = (fb_info_t*)out;
    extern struct video_mode current_mode;
    info->width  = current_mode.width;
    info->height = current_mode.height;
    info->pitch  = current_mode.pitch;
    info->bpp    = current_mode.bpp;
    return 0;
}

int32_t sys_fb_fill(uint32_t rgb, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6) {
    (void)a2; (void)a3; (void)a4; (void)a5; (void)a6;
    display_clear(rgb);
    return 0;
}

int32_t sys_fb_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t rgb, uint32_t a6) {
    (void)a6;
    display_fill_rect(x, y, w, h, rgb);
    return 0;
}
