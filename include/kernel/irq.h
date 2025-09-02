#ifndef _KERNEL_IRQ_H
#define _KERNEL_IRQ_H

#include <stdint.h>

// IRQ numbers
#define IRQ0  32
#define IRQ1  33
#define IRQ2  34
#define IRQ3  35
#define IRQ4  36
#define IRQ5  37
#define IRQ6  38
#define IRQ7  39
#define IRQ8  40
#define IRQ9  41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

// IRQ handler function pointer type
typedef void (*irq_handler_t)(void);

// Initialize the IRQ subsystem
void irq_init(void);

// Register an IRQ handler (implemented in arch-specific code)
void irq_install_handler(uint8_t irq, void (*handler)(void));

// Unregister an IRQ handler (not implemented in kernel/irq.c)
// void irq_uninstall_handler(uint8_t irq);

// IRQ handler (called from assembly) - implemented in arch-specific code
void irq_handler(uint8_t irq);

// Enable/disable interrupts
void enable_interrupts(void);
void disable_interrupts(void);

// Check if interrupts are enabled
int interrupts_enabled(void);

// Send EOI (End Of Interrupt) to PIC
void send_eoi(uint8_t irq);

#endif // _KERNEL_IRQ_H
