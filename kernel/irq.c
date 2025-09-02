#include <kernel/irq.h>
#include <kernel/console.h>
#include <arch/x86/io.h>
#include <stdint.h>

// For assembly functions
#define ASM_VOLATILE(...) __asm__ __volatile__(__VA_ARGS__)

// Initialize IRQ handlers - implemented in arch/x86/x86/interrupts/idt.c
void irq_init(void) {
    console_puts("[irq] IRQ subsystem initialized\n");
}

// Enable interrupts
void enable_interrupts(void) {
    ASM_VOLATILE("sti");
}

// Disable interrupts
void disable_interrupts(void) {
    ASM_VOLATILE("cli");
}

// Check if interrupts are enabled
int interrupts_enabled(void) {
    unsigned long flags;
    ASM_VOLATILE("pushf ; pop %0" : "=g"(flags));
    return flags & (1 << 9); // Check IF flag
}

// Send EOI (End Of Interrupt) to PIC
void send_eoi(uint8_t irq) {
    if (irq >= 8) {
        // Send EOI to slave
        outb(0xA0, 0x20);
    }
    
    // Send EOI to master
    outb(0x20, 0x20);
}
