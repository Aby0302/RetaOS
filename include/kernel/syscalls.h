#ifndef _KERNEL_SYSCALLS_H
#define _KERNEL_SYSCALLS_H

#include <stdint.h>

// Sistem çağrı numaraları
typedef enum {
    SYS_EXIT = 0,
    SYS_FORK,
    SYS_READ,
    SYS_WRITE,
    SYS_OPEN,
    SYS_CLOSE,
    SYS_EXECVE,
    SYS_WAITPID,
    SYS_GETPID,
    SYS_GETPPID,
    SYS_SBRK,
    SYS_MMAP,
    SYS_MUNMAP,
    SYS_STAT,
    SYS_LSEEK,
    SYS_IOCTL,
    SYS_ACCESS,
    SYS_DUP2,
    SYS_GETDENTS,
    SYS_GETCWD,
    SYS_CHDIR,
    SYS_FSTAT,
    SYS_FCNTL,
    SYS_READDIR,
    SYS_CLONE,
    SYS_EXIT_GROUP,
    SYS_KILL,
    SYS_NANOSLEEP,
    SYS_GETTIMEOFDAY,
    SYS_GETUID,
    SYS_GETGID,
    SYS_GETEUID,
    SYS_GETEGID,
    SYS_SETUID,
    SYS_SETGID,
    SYS_SETSID,
    SYS_GETSID,
    SYS_GETPGID,
    SYS_SETPGID,
    SYS_UMASK,
    SYS_UNAME,
    SYS_TIMES,
    SYS_BRK,
    SYS_MKDIR,
    SYS_RMDIR,
    SYS_UNLINK,
    SYS_LINK,
    SYS_SYMLINK,
    SYS_READLINK,
    SYS_CHMOD,
    SYS_FCHMOD,
    SYS_CHOWN,
    SYS_FCHOWN,
    SYS_LCHOWN,
    SYS_GETPRIORITY,
    SYS_SETPRIORITY,
    SYS_SYSCONF,
    SYS_GETRUSAGE,
    SYS_GETRLIMIT,
    SYS_SETRLIMIT,
    SYS_GETGROUPS,
    SYS_SETGROUPS,
    SYS_SOCKETCALL,
    SYS_MMAP2,
    SYS_TRUNCATE,
    SYS_FTRUNCATE,
    SYS_FCHDIR,
    SYS_FSYNC,
    SYS_FDATASYNC,
    SYS_FCNTL64,
    SYS_SET_TID_ADDRESS,
    SYS_CLOCK_GETTIME,
    SYS_CLOCK_GETRES,
    SYS_CLOCK_NANOSLEEP,
    SYS_PREAD64,
    SYS_PWRITE64,
    SYS_GETTID,
    SYS_FUTEX,
    SYS_SCHED_SETAFFINITY,
    SYS_SCHED_GETAFFINITY,
    SYS_IO_SETUP,
    SYS_IO_DESTROY,
    SYS_IO_GETEVENTS,
    SYS_IO_SUBMIT,
    SYS_IO_CANCEL,
    SYS_EPOLL_CREATE,
    SYS_EPOLL_CTL,
    SYS_EPOLL_WAIT,
    SYS_REMAP_FILE_PAGES,
    SYS_SET_THREAD_AREA,
    SYS_GET_THREAD_AREA,
    SYS_EPOLL_CREATE1,
    SYS_EPOLL_PWAIT,
    SYS_OPENAT,
    SYS_MKDIRAT,
    SYS_MKNODAT,
    SYS_FCHOWNAT,
    SYS_FUTIMESAT,
    SYS_NEWFSTATAT,
    SYS_UNLINKAT,
    SYS_RENAMEAT,
    SYS_LINKAT,
    SYS_SYMLINKAT,
    SYS_READLINKAT,
    SYS_FCHMODAT,
    SYS_FACCESSAT,
    SYS_PIPE2,
    SYS_DUP3,
    SYS_EPOLL_CREATE2,
    SYS_PREADV,
    SYS_PWRITEV,
    SYS_RT_TGSIGQUEUEINFO,
    SYS_PERF_EVENT_OPEN,
    SYS_RECVMMSG,
    SYS_SENDMMSG,
    SYS_GETDENTS64,
    // --- RetaOS custom extensions for GUI/FB (explicit values) ---
    SYS_FB_GETINFO = 240,
    SYS_FB_FILL    = 241,
    SYS_FB_RECT    = 242,
    SYS_COUNT // Toplam syscall sayısı
} syscall_num_t;

// Sistem çağrısı işleyici fonksiyon prototipi
typedef int32_t (*syscall_handler_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3, 
                                   uint32_t arg4, uint32_t arg5, uint32_t arg6);

// Sistem çağrısını kaydet
void syscall_register(uint32_t num, syscall_handler_t handler);

// Sistem çağrısını işle
int32_t syscall_dispatch(uint32_t num, uint32_t arg1, uint32_t arg2, 
                        uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6);

// Sistem çağrılarını başlat
void syscalls_init(void);

// Kullanıcı modundan sistem çağrısı yapmak için yardımcı makrolar
#define SYSCALL0(num) ({ \
    int32_t ret; \
    asm volatile ("int $0x80" : "=a" (ret) : "a" (num) : "memory"); \
    ret; \
})

#define SYSCALL1(num, a1) ({ \
    int32_t ret; \
    asm volatile ("int $0x80" : "=a" (ret) : "a" (num), "b" ((uint32_t)(a1)) : "memory"); \
    ret; \
})

#define SYSCALL2(num, a1, a2) ({ \
    int32_t ret; \
    asm volatile ("int $0x80" : "=a" (ret) : "a" (num), "b" ((uint32_t)(a1)), "c" ((uint32_t)(a2)) : "memory"); \
    ret; \
})

#define SYSCALL3(num, a1, a2, a3) ({ \
    int32_t ret; \
    asm volatile ("int $0x80" : "=a" (ret) : "a" (num), "b" ((uint32_t)(a1)), \
                  "c" ((uint32_t)(a2)), "d" ((uint32_t)(a3)) : "memory"); \
    ret; \
})

#define SYSCALL4(num, a1, a2, a3, a4) ({ \
    int32_t ret; \
    register uint32_t _a4 asm("esi") = (uint32_t)(a4); \
    asm volatile ("int $0x80" : "=a" (ret) : "a" (num), "b" ((uint32_t)(a1)), \
                  "c" ((uint32_t)(a2)), "d" ((uint32_t)(a3)), "S" (_a4) : "memory"); \
    ret; \
})

#define SYSCALL5(num, a1, a2, a3, a4, a5) ({ \
    int32_t ret; \
    register uint32_t _a4 asm("esi") = (uint32_t)(a4); \
    register uint32_t _a5 asm("edi") = (uint32_t)(a5); \
    asm volatile ("int $0x80" : "=a" (ret) : "a" (num), "b" ((uint32_t)(a1)), \
                  "c" ((uint32_t)(a2)), "d" ((uint32_t)(a3)), "S" (_a4), "D" (_a5) : "memory"); \
    ret; \
})

#define SYSCALL6(num, a1, a2, a3, a4, a5, a6) ({ \
    int32_t ret; \
    register uint32_t _a4 asm("esi") = (uint32_t)(a4); \
    register uint32_t _a5 asm("edi") = (uint32_t)(a5); \
    register uint32_t _a6 asm("ebp") = (uint32_t)(a6); \
    asm volatile ("push %%ebp\n" \
                  "mov %7, %%ebp\n" \
                  "int $0x80\n" \
                  "pop %%ebp" \
                  : "=a" (ret) \
                  : "a" (num), "b" ((uint32_t)(a1)), "c" ((uint32_t)(a2)), \
                    "d" ((uint32_t)(a3)), "S" (_a4), "D" (_a5), "g" (_a6) \
                  : "memory"); \
    ret; \
})

#endif // _KERNEL_SYSCALLS_H
