#ifndef _KERNEL_SCHED_H
#define _KERNEL_SCHED_H

#include <stdint.h>
#include <kernel/thread.h>
#include <kernel/process.h>
#include <kernel/scheduler.h>  // Include scheduler interface

// Scheduler functions
void sched_init(void);
void sched_start(void);

// Thread scheduling
void sched_add_thread(thread_t* thread);
void sched_remove_thread(thread_t* thread);
void sched_yield(void);
thread_t* sched_current_thread(void);
void sched_switch(thread_t* next);
void sched_add_sleeping_thread(thread_t* thread);

// Process scheduling
void sched_add_process(process_t* proc);
void sched_remove_process(process_t* proc);
process_t* sched_current_process(void);

// Context switching
void switch_threads(thread_t* from, thread_t* to);

// Timer tick handler (called from IRQ0)
void sched_tick(void);

#endif // _KERNEL_SCHED_H
