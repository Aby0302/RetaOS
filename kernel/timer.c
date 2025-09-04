#include <kernel/timer.h>
#include <kernel/irq.h>
#include <kernel/console.h>
// No need for console_utils.h since we'll use console_puts
#include <arch/x86/io.h>
#include <stdint.h>

// For assembly functions
#define ASM_VOLATILE(...) __asm__ __volatile__(__VA_ARGS__)

// Global tick counter (declared in header, defined here)
volatile uint32_t timer_ticks = 0;

// Timer interrupt handler callback
static void (*timer_callback)(void) = 0;

// Set the timer phase (implementation)
void timer_phase(int hz) {
    int divisor = 1193180 / hz;       // Calculate the divisor
    outb(0x43, 0x36);                // Set command byte 0x36 (channel 0, lobyte/hibyte, mode 3, binary)
    outb(0x40, divisor & 0xFF);      // Set low byte of divisor
    outb(0x40, (divisor >> 8) & 0xFF); // Set high byte of divisor
}

// Timer interrupt handler
void timer_handler(void) {
    timer_ticks++;
    
    // Call the registered callback if it exists
    if (timer_callback) {
        timer_callback();
    }
}

// Initialize the timer with the specified frequency
void timer_init(uint32_t frequency) {
    // Register the timer handler for IRQ0
    irq_install_handler(0, timer_handler);
    
    // Set the timer phase
    timer_phase(frequency);
    
    // Simple message since we can't format strings easily
    console_puts("[timer] Timer initialized\n");
}

// Get the current tick count
uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

// Wait for the specified number of ticks
void timer_wait(uint32_t ticks) {
    uint32_t eticks = timer_ticks + ticks;
    while (timer_ticks < eticks) {
        // Wait for the tick count to reach the desired value
        ASM_VOLATILE("hlt");
    }
}

// Sleep for the specified number of milliseconds
void kernel_sleep(uint32_t ms) {
    // Calculate the number of ticks to wait
    uint32_t ticks = (ms * TIMER_FREQ) / 1000;
    if (ticks == 0) {
        ticks = 1; // Sleep for at least 1 tick
    }

    uint32_t end_ticks = timer_ticks + ticks;
    while (timer_ticks < end_ticks) {
        ASM_VOLATILE("hlt");
    }
}

// Set a callback function to be called on each timer tick
void timer_set_callback(void (*callback)(void)) {
    timer_callback = callback;
}
