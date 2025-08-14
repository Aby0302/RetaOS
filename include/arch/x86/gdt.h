#pragma once
void gdt_init(void);
void tss_set_kernel_stack(unsigned int esp0);
