#pragma once
#include <stdint.h>

// Saved general-purpose registers as laid out by pusha/pushal
// Order matches the stack layout after pushal: EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
struct isr_context {
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp; // original ESP at exception entry (before pushal)
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
};

void exception_handler(uint32_t vector, uint32_t error_code, const struct isr_context* ctx);
void irq_handler(uint8_t irq);
void irq_install_handler(uint8_t irq, void (*handler)(void));
void irq_init_basic(void);

// Panic interface
void panic(const char* message);

// Timer ticks since boot (PIT at 100Hz)
unsigned int timer_get_ticks(void);

// IRQ0 handler for timer interrupts
void irq0_handler(uint32_t isr_esp);
