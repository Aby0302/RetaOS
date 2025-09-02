#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

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
    // Diğer syscall'lar...
} syscall_num_t;

// Syscall makroları
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

#endif // _SYS_SYSCALL_H
