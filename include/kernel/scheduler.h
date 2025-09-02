#ifndef _KERNEL_SCHEDULER_H
#define _KERNEL_SCHEDULER_H

#include <kernel/thread.h>
#include <kernel/process.h>

// Scheduler interface
extern void scheduler_init(void);
extern void scheduler_start(void);
extern void scheduler_set_preempt(int on);
extern int scheduler_get_preempt(void);

// Thread scheduling
extern void sched_add_thread(thread_t* thread);
extern void sched_remove_thread(thread_t* thread);
extern void sched_yield(void);
extern thread_t* sched_current_thread(void);

// Process scheduling
extern void sched_add_process(process_t* proc);
extern void sched_remove_process(process_t* proc);
extern process_t* sched_current_process(void);

// Timer tick handler
extern void sched_tick(void);

// Timer interrupt service routine for the scheduler
extern void scheduler_on_timer_isr(void);

// Sleep/wake functionality
extern void sched_add_sleeping_thread(thread_t* thread);

#endif // _KERNEL_SCHEDULER_H
