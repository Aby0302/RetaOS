#pragma once
#include <stdint.h>
void isr14_handler(void);
void irq_handler(uint8_t irq);
void irq_install_handler(uint8_t irq, void (*handler)(void));
void irq_init_basic(void);
