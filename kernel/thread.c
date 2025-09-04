#include <kernel/thread.h>
#include <kernel/kheap.h>
#include <kernel/sched.h>
#include <kernel/process.h>
#include <kernel/timer.h>
#include <kernel/console.h>
#include <arch/x86/io.h>
#include <string.h>

// For assembly functions
#define ASM_VOLATILE(...) __asm__ __volatile__(__VA_ARGS__)

// Forward declarations
#include <kernel/bsod.h>
void panic(const char* msg);
void kprintf(const char* fmt, ...);

// Thread entry point
static void thread_entry(void);

// Timer ticks per second (should match the value in timer.h)
#ifndef TICKS_PER_SEC
#define TICKS_PER_SEC 100
#endif

#define DEFAULT_STACK_SIZE (16 * 1024) // 16KB stack

static tid_t next_tid = 1;
static thread_t* thread_list = NULL;
thread_t* current_thread = NULL;

// Function declarations
static void thread_entry(void);
static void setup_thread_stack(thread_t* thread);
tid_t thread_create(void* (*entry)(void*), void* arg);
void thread_exit(void* retval);
int thread_join(tid_t tid, void** retval);
void thread_yield(void);
tid_t thread_self(void);
void thread_sleep(uint32_t ms);
void threading_init(void);

// Thread entry point
static void thread_entry(void) {
    // Debug output
    extern void serial_write(const char* str);
    serial_write("[DEBUG] thread_entry called!\r\n");
    
    // Get current thread from scheduler
    extern thread_t* sched_current_thread(void);
    thread_t* thread = sched_current_thread();
    
    if (!thread) {
        serial_write("[DEBUG] ERROR: current_thread is NULL!\r\n");
        return;
    }
    
    serial_write("[DEBUG] About to call thread function\r\n");
    
    // Call thread function
    void* retval = thread->entry(thread->arg);
    
    // Thread exit
    thread_exit(retval);
}

// Helper function to initialize thread stack
static void setup_thread_stack(thread_t* thread) {
    // Set up initial stack frame for thread
    uint32_t* stack_top = (uint32_t*)((uint8_t*)thread->stack_base + thread->stack_size);
    
    // Align to 16 bytes
    stack_top = (uint32_t*)((uint32_t)stack_top & ~0xF);
    
    // For ret instruction, we just need the return address on stack
    *--stack_top = (uint32_t)thread_entry;  // Return address for ret
    
    // Push initial register values for switch_to_thread
    *--stack_top = 0;  // EDI
    *--stack_top = 0;  // ESI
    *--stack_top = 0;  // EBX
    *--stack_top = 0;  // EBP
    
    thread->stack = stack_top;
}

// Create a new thread
tid_t thread_create(void* (*entry)(void*), void* arg) {
    if (!entry) return -1;
    
    // Allocate thread structure
    thread_t* thread = (thread_t*)kmalloc(sizeof(thread_t));
    if (!thread) return -1;
    
    // Allocate stack
    thread->stack_size = DEFAULT_STACK_SIZE;
    thread->stack_base = kmalloc(thread->stack_size);
    if (!thread->stack_base) {
        kfree(thread);
        return -1;
    }
    
    // Initialize thread
    thread->tid = next_tid++;
    thread->state = THREAD_READY;
    thread->entry = entry;
    thread->arg = arg;
    thread->retval = NULL;
    thread->process = process_current();
    
    // Set up stack
    setup_thread_stack(thread);
    
    // Add to thread list
    thread->next = thread_list;
    thread_list = thread;
    
    // Add to scheduler
    sched_add_thread(thread);
    
    return thread->tid;
}

// Exit current thread
void thread_exit(void* retval) {
    if (!current_thread) return;
    
    // Save return value
    current_thread->retval = retval;
    current_thread->state = THREAD_TERMINATED;
    
    // Schedule next thread
    thread_yield();
    
    // Should never reach here
    for(;;) ASM_VOLATILE("hlt");
}

// Wait for a thread to finish
int thread_join(tid_t tid, void** retval) {
    if (tid <= 0) return -1;
    
    // Find the thread
    thread_t* thread = thread_list;
    thread_t* prev = NULL;
    
    while (thread) {
        if (thread->tid == tid) {
            break;
        }
        prev = thread;
        thread = thread->next;
    }
    
    // Thread not found
    if (!thread) return -1;
    
    // Can't join self
    if (thread == current_thread) return -1;
    
    // Wait until thread terminates
    while (thread->state != THREAD_TERMINATED) {
        thread_yield();
    }
    
    // Return thread's return value if requested
    if (retval) {
        *retval = thread->retval;
    }
    
    // Remove from thread list
    if (prev) {
        prev->next = thread->next;
    } else {
        thread_list = thread->next;
    }
    
    // Free thread resources
    kfree(thread->stack_base);
    kfree(thread);
    
    return 0;
}

// Yield CPU to another thread
void thread_yield(void) {
    ASM_VOLATILE("int $0x20"); // Yield interrupt
}

// Get current thread ID
tid_t thread_self(void) {
    return current_thread ? current_thread->tid : 0;
}

// Sleep for milliseconds
void thread_sleep(uint32_t ms) {
    if (ms == 0) {
        thread_yield();
        return;
    }
    
    // Calculate wakeup time (assuming 1000 ticks per second)
    uint32_t wakeup_time = timer_ticks + (ms * 1000) / TICKS_PER_SEC;
    
    // Set thread state to blocked
    current_thread->state = THREAD_BLOCKED;
    current_thread->wakeup_time = wakeup_time;
    
    // Add to sleep queue
    sched_add_sleeping_thread(current_thread);
    
    // Yield CPU
    thread_yield();
}

// Initialize threading system
void threading_init(void) {
    // Create main thread
    current_thread = (thread_t*)kmalloc(sizeof(thread_t));
    if (!current_thread) {
        kernel_bsod("Failed to initialize main thread");
    }
    
    // Initialize main thread
    memset(current_thread, 0, sizeof(thread_t));
    current_thread->tid = next_tid++;
    // Bootstrap thread represents the current kernel context; mark as RUNNING
    current_thread->state = THREAD_RUNNING;
    current_thread->process = process_current();
    current_thread->time_slice = rr_quantum;
    
    // Allocate stack for main thread
    current_thread->stack_size = DEFAULT_STACK_SIZE;
    current_thread->stack_base = kmalloc(current_thread->stack_size);
    if (!current_thread->stack_base) {
        kfree(current_thread);
        kernel_bsod("Failed to allocate stack for main thread");
    }
    
    // Set up initial stack pointer
    current_thread->stack = (void*)((uint8_t*)current_thread->stack_base + current_thread->stack_size - 16);
    
    // Add to thread list
    thread_list = current_thread;
    
    // Do NOT enqueue bootstrap thread into ready queue; scheduler will start
    // with the first real kernel/user thread created later.
    console_puts("[thread] Threading system initialized\n");
}

// Expose head of thread list for diagnostics
thread_t* thread_list_head(void) { return thread_list; }
