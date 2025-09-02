// Advanced scheduler with thread support
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <kernel/sched.h>
#include <kernel/timer.h>  // For timer_phase
#include <kernel/console.h> // For kprintf
#include <kernel/scheduler.h>
#include <kernel/thread.h>
#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/irq.h>
#include <kernel/console.h>
#include <arch/x86/io.h>
#include <kernel/kheap.h>

// Forward declarations
void console_puts(const char *str);
void timer_phase(int hz);
void switch_to_thread(thread_t* thread);
void kprintf(const char* fmt, ...);
void serial_write(const char* str);
#include <arch/x86/gdt.h>
#include <kernel/pmm.h>

// External assembly functions
extern void switch_threads(thread_t* from, thread_t* to);

// Global variables
static thread_t* ready_queue = NULL;
static thread_t* sleeping_threads = NULL;  // List of sleeping threads
static thread_t* current_thread = NULL;
static process_t* process_list = NULL;
static process_t* current_process = NULL;
static int sched_initialized = 0;
static int preempt_enabled = 1;
int rr_quantum = 5; // Time slice in timer ticks

// Global counters for thread and process IDs
static tid_t next_tid = 1;
static pid_t next_pid = 1;

// Thread scheduling functions
void sched_add_thread(thread_t* thread) {
    if (!thread) return;
    
    // Add to the front of the ready queue
    thread->next = ready_queue;
    ready_queue = thread;
}

void sched_remove_thread(thread_t* thread) {
    if (!thread || !ready_queue) return;
    
    // If it's the first thread in the queue
    if (ready_queue == thread) {
        ready_queue = ready_queue->next;
        return;
    }
    
    // Find the thread in the queue
    thread_t* prev = ready_queue;
    while (prev->next && prev->next != thread) {
        prev = prev->next;
    }
    
    // Remove the thread if found
    if (prev->next == thread) {
        prev->next = thread->next;
    }
}

// Get the next thread to run
static thread_t* pick_next_thread(void) {
    if (!ready_queue) return NULL;
    
    // Simple round-robin: move head to tail
    thread_t* next = ready_queue;
    ready_queue = ready_queue->next;
    next->next = NULL;
    
    return next;
}

// Add a thread to the sleeping threads list
void sched_add_sleeping_thread(thread_t* thread) {
    if (!thread) return;
    
    thread->next_sleeping = sleeping_threads;
    sleeping_threads = thread;
}

// Check and wake up sleeping threads
static void check_sleeping_threads(void) {
    thread_t** ptr = &sleeping_threads;
    thread_t* current = sleeping_threads;
    
    while (current) {
        if (current->wakeup_time <= timer_ticks) {
            // Wake up the thread
            *ptr = current->next_sleeping;
            current->state = THREAD_READY;
            sched_add_thread(current);
            current = *ptr;
        } else {
            ptr = &current->next_sleeping;
            current = current->next_sleeping;
        }
    }
}

// Timer tick handler - called from timer interrupt
void sched_tick(void) {
    if (!current_thread) return;
    
    // Decrement the time slice
    current_thread->time_slice--;
    
    // Check if the time slice has expired
    if (current_thread->time_slice <= 0) {
        // Reset the time slice
        current_thread->time_slice = rr_quantum;
        
        // If preemption is enabled, yield the CPU
        if (preempt_enabled) {
            sched_yield();
        }
    }
    
    // Check for sleeping threads that need to be woken up
    check_sleeping_threads();
}

// Yield the CPU to another thread
void sched_yield(void) {
    if (!sched_initialized) return;
    
    // Get next thread to run
    thread_t* next = pick_next_thread();
    if (!next) return; // No other threads to run
    
    // Get current thread
    thread_t* current = current_thread;
    
    // If current thread is still runnable, add it back to the ready queue
    if (current && current->state == THREAD_RUNNING) {
        current->state = THREAD_READY;
        sched_add_thread(current);
    }
    
    // Switch to the next thread
    next->state = THREAD_RUNNING;
    next->time_slice = rr_quantum;
    
    // Update current thread
    thread_t* prev_thread = current_thread;
    current_thread = next;
    
    // If this is the first thread, we don't need to switch
    if (!prev_thread) {
        return;
    }
    
    // Perform the context switch
    switch_threads(prev_thread, next);
}

// Initialize the scheduler
void sched_init(void) {
    // Initialize the ready queue
    ready_queue = NULL;
    
    // Initialize the sleeping threads list
    sleeping_threads = NULL;
    
    // Initialize the process list
    process_list = NULL;
    
    // No current thread or process yet
    current_thread = NULL;
    current_process = NULL;
    
    // Initialize the task ID counter
    next_tid = 1;
    next_pid = 1;
    
    // Set up the timer
    timer_phase(100); // 100 Hz
    
    // Enable preemption by default
    preempt_enabled = 1;
    
    // Mark scheduler as initialized
    sched_initialized = 1;
    
    //kprintf("[sched] Scheduler initialized\n");
}

// Alias for scheduler_init for backward compatibility
void scheduler_init(void) {
    sched_init();
}

// Alias for scheduler_start for backward compatibility
void scheduler_start(void) {
    sched_start();
}

// Start the scheduler
void sched_start(void) {
    // Make sure we have at least one thread
    if (!ready_queue) {
        serial_write("[DEBUG] No threads to schedule!\r\n");
        return;
    }
    
    // Get the first thread to run
    thread_t* next = ready_queue;
    ready_queue = ready_queue->next;
    
    serial_write("[DEBUG] Starting scheduler with first thread\r\n");
    
    // Mark it as running
    next->state = THREAD_RUNNING;
    current_thread = next;
    
    // Update TSS
    tss_set_kernel_stack((uint32_t)next->stack_base + next->stack_size);
    
    serial_write("[DEBUG] About to switch to first thread\r\n");
    
    // Switch to the first thread
    switch_to_thread(next);
    
    // We should never get here
    serial_write("[DEBUG] Scheduler returned unexpectedly!\r\n");
    for (;;) __asm__ __volatile__ ("hlt");
}

// Get current thread
thread_t* sched_current_thread(void) {
    return current_thread;
}

// Get current process
process_t* sched_current_process(void) {
    return current_process;
}

// Timer interrupt service routine for the scheduler
void scheduler_on_timer_isr(void) {
    sched_tick();
}

// Process management functions
void sched_add_process(process_t* proc) {
    if (!proc) return;
    
    // Add to process list
    proc->next = process_list;
    process_list = proc;
    
    // If this is the first process, set it as current
    if (!current_process) {
        current_process = proc;
    }
}

void sched_remove_process(process_t* proc) {
    if (!proc || !process_list) return;
    
    // If it's the first process
    if (process_list == proc) {
        process_list = proc->next;
        return;
    }
    
    // Find the process in the list
    process_t* prev = process_list;
    while (prev->next && prev->next != proc) {
        prev = prev->next;
    }
    
    // Remove the process if found
    if (prev->next == proc) {
        prev->next = proc->next;
    }
    
    // If we're removing the current process, update current_process
    if (current_process == proc) {
        current_process = process_list; // Fall back to first process
    }
}

// Switch to a specific thread (used by thread_yield)
void sched_switch(thread_t* next) {
    if (!next || next->state != THREAD_READY) return;
    
    thread_t* current = current_thread;
    if (current == next) return;
    
    // Update states
    if (current) {
        current->state = THREAD_READY;
        sched_add_thread(current);
    }
    
    next->state = THREAD_RUNNING;
    next->time_slice = rr_quantum;
    current_thread = next;
    
    // Update TSS
    tss_set_kernel_stack((uint32_t)next->stack_base + next->stack_size);
    
    // Perform the context switch
    switch_threads(current, next);
}

// Assembly function to switch to a thread
void switch_to_thread(thread_t* thread) {
    if (!thread) return;
    
    serial_write("[DEBUG] Switching to thread\r\n");
    
    // For kernel threads, we can use a simpler approach
    // The stack is set up with: EBP, EBX, ESI, EDI, dummy_ret, EIP, CS, EFLAGS
    __asm__ __volatile__ (
        "mov %0, %%esp\n\t"     // Load new stack pointer
        "pop %%edi\n\t"         // Restore EDI
        "pop %%esi\n\t"         // Restore ESI  
        "pop %%ebx\n\t"         // Restore EBX
        "pop %%ebp\n\t"         // Restore EBP
        "ret"                   // Return to thread_entry
        :
        : "r" (thread->stack)
        : "memory"
    );
}

// Set preemption
void scheduler_set_preempt(int on) {
    preempt_enabled = (on != 0);
}

// Get preemption status
int scheduler_get_preempt(void) {
    return preempt_enabled;
}
