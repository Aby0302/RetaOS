#ifndef _KERNEL_THREAD_H
#define _KERNEL_THREAD_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/types.h>

// Thread states
typedef enum {
    THREAD_NEW,
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_TERMINATED
} thread_state_t;

// Thread control block
typedef struct thread {
    tid_t tid;                  // Thread ID
    void* stack;                // Thread stack pointer
    void* stack_base;           // Stack base address
    size_t stack_size;          // Stack size
    thread_state_t state;       // Thread state
    void* (*entry)(void*);      // Thread entry function
    void* arg;                  // Thread argument
    void* retval;               // Return value
    struct process* process;    // Parent process
    struct thread* next;        // Next thread in list
    struct thread* next_sleeping; // Next thread in sleeping list
    uint32_t wakeup_time;       // Time when thread should wake up
    int time_slice;             // Remaining time slice
} thread_t;

// Thread functions
tid_t thread_create(void* (*entry)(void*), void* arg);
void thread_exit(void* retval);
int thread_join(tid_t tid, void** retval);
void thread_yield(void);
tid_t thread_self(void);
void thread_sleep(uint32_t ms);

// Threading initialization
void threading_init(void);

// Round-robin quantum (defined in sched.c)
extern int rr_quantum;

// Introspection helper: return head of internal thread list
// Used by diagnostics (e.g., ps command)
thread_t* thread_list_head(void);

#endif // _KERNEL_THREAD_H
