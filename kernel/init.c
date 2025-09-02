#include <arch/x86/multiboot2.h>
#include <kernel/display.h>
#include "drivers/input/event.h"
#include <kernel/console.h>

// Kernel initialization entry point
void kernel_init(unsigned long magic, struct multiboot_info *mbi) {
    // Initialize display subsystem
    display_init(magic, (uintptr_t)mbi);

    // Initialize input subsystem
    input_init();

    // Initialize other core systems (e.g., memory, process, etc.)
    // You may need to add additional initialization calls here
    console_init();
}

// Ensure this is called from the boot code (e.g., boot2.S or bootloader)
