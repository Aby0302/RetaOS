#pragma once
#include <stdint.h>

typedef void (*thread_entry_t)(void*);

struct task {
    // Saved context
    uint32_t esp;    // stack pointer
    uint32_t eip;    // resume address

    // Kernel stack
    uint32_t kstack_base;   // base address returned by allocator
    uint32_t kstack_top;    // top (esp0) for TSS

    // Thread info
    thread_entry_t entry;
    void*          arg;
    int            id;
    int            state;         // 0=ready, 1=running
    int            time_slice;    // remaining ticks for RR
    // ISR preemption support
    uint32_t       isr_esp;       // stack ptr to popal area for ISR-exit switch
    int            has_isr_frame; // 1 if isr_esp is valid
};

void scheduler_init(void);
int  create_kernel_thread(thread_entry_t entry, void* arg);
void scheduler_tick(void);   // cooperative switch (call from threads)
void scheduler_on_timer(void); // called from PIT interrupt to request switch
// ISR-based preemption path: called from timer interrupt
void scheduler_on_timer_isr(void);
void yield(void);
void scheduler_start(void);  // switch to first task

// Optional: adjust global quantum (ticks per task)
void scheduler_set_quantum(int ticks);

// Diagnostics helpers
int scheduler_task_count(void);
int scheduler_current_index(void);
int scheduler_get_quantum(void);

// Preemption control (1=on, 0=off)
void scheduler_set_preempt(int on);
int  scheduler_get_preempt(void);
