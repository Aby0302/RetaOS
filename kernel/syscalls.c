#include <kernel/syscalls.h>
#include <kernel/process.h>
#include <kernel/vfs.h>
#include <memory/heap.h>
#include <kernel/elf.h>
#include <kernel/sched.h>
#include <drivers/serial.h>  // serial_write için
#include <arch/x86/isr.h>    // isr_context yapısı için
#include <stddef.h>
#include <stdint.h>
#include <string.h>

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
    
    // TODO: Dosya tanıtıcıları için kontrol ekle
    if (fd == 0) { // stdin
        // Klavyeden okuma
        return 0; // Şimdilik desteklenmiyor
    }
    
    // Dosyadan okuma
    return vfs_read(fd, buf, count);
}

// write - Dosyaya yazma
static int32_t sys_write(int fd, const void* buf, size_t count, 
                        uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1; (void)unused2; (void)unused3;
    
    if (fd == 1 || fd == 2) { // stdout veya stderr
        // Ekrana yaz
        const char* str = (const char*)buf;
        for (size_t i = 0; i < count; i++) {
            if (str[i] == '\0') break;
            char temp[2] = {str[i], '\0'};
            serial_write(temp);
        }
        return count;
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
    
    serial_write("[syscalls] Initialized system calls\n");
}
