#ifndef _SETJMP_H
#define _SETJMP_H

#include <stddef.h>

// Buffer for holding processor state
#if defined(__i386__)
typedef struct {
    unsigned long __ebx;
    unsigned long __esi;
    unsigned long __edi;
    unsigned long __ebp;
    unsigned long __esp;
    unsigned long __eip;
} jmp_buf[1];
#else
#error "setjmp.h: Architecture not supported"
#endif

// Function declarations
int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

// For C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

// POSIX sigsetjmp/siglongjmp (simplified)
typedef jmp_buf sigjmp_buf;
#define sigsetjmp(env, savesigs) setjmp(env)
#define siglongjmp(env, val) longjmp(env, val)

#ifdef __cplusplus
}
#endif

#endif /* _SETJMP_H */
