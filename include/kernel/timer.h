#ifndef _KERNEL_TIMER_H
#define _KERNEL_TIMER_H

#include <stdint.h>

// Timer frequency in Hz
#define TIMER_FREQ 100

// Global tick counter (defined in timer.c)
extern volatile uint32_t timer_ticks;

// Set the timer phase (clock rate)
void timer_phase(int hz);

// Initialize the timer
void timer_init(uint32_t frequency);

// Wait for the specified number of ticks
void timer_wait(uint32_t ticks);

// Get the current tick count
uint32_t timer_get_ticks(void);

// Timer interrupt handler (called from ISR)
void timer_handler(void);

// Sleep for the specified number of milliseconds
void sleep(uint32_t ms);

// Set a callback function to be called on each timer tick
void timer_set_callback(void (*callback)(void));

#endif // _KERNEL_TIMER_H
