#ifndef _KERNEL_BSOD_H
#define _KERNEL_BSOD_H

#include <stdint.h>
#include "../arch/x86/isr.h"

// Basit panic makrosu ve assert
#define KERNEL_PANIC(msg) do { kernel_bsod("%s", (msg)); } while(0)
#define KASSERT(cond, msg) do { if (!(cond)) kernel_bsod("ASSERT FAILED: %s", (msg)); } while(0)

// Basit mavi ekran
void kernel_bsod(const char* msg, ...);

// Istisnalar icin detayli mavi ekran
void kernel_bsod_exception(uint32_t vector, uint32_t error_code, const struct isr_context* ctx, uint32_t cr2);

#endif
