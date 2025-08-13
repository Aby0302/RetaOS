#include "include/drivers/serial.h"
#include "include/multiboot.h"
#include "include/memory/pmm.h"

extern uint32_t _kernel_start; // from linker
extern uint32_t _kernel_end;   // from linker

static void log_hex(const char* pfx, uint32_t v){ serial_write(pfx); serial_write_hex(v); serial_write("\n"); }

void mb_parse_and_init(uint32_t multiboot_magic, uint32_t multiboot_info_addr){
    if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC){
        serial_write("[MB] Invalid multiboot magic!\n");
        return;
    }
    multiboot_info_t* mbi = (multiboot_info_t*)(uintptr_t)multiboot_info_addr;
    serial_write("[MB] flags="); serial_write_hex(mbi->flags); serial_write("\n");
    serial_write("[MB] mem_lower="); serial_write_dec(mbi->mem_lower); serial_write(" KB\n");
    serial_write("[MB] mem_upper="); serial_write_dec(mbi->mem_upper); serial_write(" KB\n");

    if (!(mbi->flags & (1 << 6))){
        serial_write("[MB] mmap not provided by bootloader.\n");
    }

    // Initialize Physical Memory Manager with basic info and full mmap
    pmm_init(mbi->mem_upper, (uint32_t)(uintptr_t)&_kernel_start, (uint32_t)(uintptr_t)&_kernel_end, mbi->mmap_addr, mbi->mmap_length);

    // Log kernel range
    log_hex("[MB] kernel_start=", (uint32_t)(uintptr_t)&_kernel_start);
    log_hex("[MB] kernel_end=", (uint32_t)(uintptr_t)&_kernel_end);
} 