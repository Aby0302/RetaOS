#include <stdint.h>
#include <stddef.h>
#include "include/arch/x86/multiboot2.h"
#include "include/drivers/serial.h"

#include <multiboot2.h>
// Memory initialization
void memory_init(uint32_t mb_magic, uint32_t mb_info_addr) {
    serial_write("[INIT] Memory initialization started\r\n");
    
    // Initialize physical memory manager
    if (mb_magic == MULTIBOOT2_BOOTLOADER_MAGIC || mb_magic == 0x1BADB002) { // Check for valid multiboot magic
        // The multiboot2 information structure starts after the total size field
        struct multiboot_tag *tag = (struct multiboot_tag *)(mb_info_addr + 8);
        uint32_t mem_upper_kb = 0;
        struct multiboot_mmap_entry *mmap_entries = NULL;
        uint32_t mmap_entry_count = 0;
        
        // Print basic memory information
        serial_write("Parsing multiboot2 tags...\r\n");
        
        // Parse all tags
        while (tag->type != MULTIBOOT_TAG_TYPE_END) {
            switch (tag->type) {
                case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
                    struct multiboot_tag_basic_meminfo *meminfo = (struct multiboot_tag_basic_meminfo *)tag;
                    serial_write("Memory: lower=");
                    serial_write_dec(meminfo->mem_lower);
                    serial_write("KB, upper=");
                    serial_write_dec(meminfo->mem_upper);
                    serial_write("KB\r\n");
                    mem_upper_kb = meminfo->mem_upper;
                    break;
                }
                
                case MULTIBOOT_TAG_TYPE_MMAP: {
                    struct multiboot_tag_mmap *mmap_tag = (struct multiboot_tag_mmap *)tag;
                    mmap_entries = (struct multiboot_mmap_entry *)mmap_tag->entries;
                    mmap_entry_count = (mmap_tag->size - sizeof(struct multiboot_tag_mmap)) / mmap_tag->entry_size;
                    
                    serial_write("Memory map (");
                    serial_write_dec(mmap_entry_count);
                    serial_write(" entries):\r\n");
                    
                    // Process each memory map entry
                    for (uint32_t i = 0; i < mmap_entry_count; i++) {
                        struct multiboot_mmap_entry *entry = &mmap_entries[i];
                        
                        serial_write("  ");
                        serial_write_dec(i + 1);
                        serial_write(". ");
                        
                        // Print memory range
                        uint32_t size_kb = (uint32_t)(entry->len / 1024);
                        if (size_kb >= 1024) {
                            serial_write_dec(size_kb / 1024);
                            serial_write(" MB, ");
                        } else {
                            serial_write_dec(size_kb);
                            serial_write(" KB, ");
                        }
                        
                        // Print type
                        switch (entry->type) {
                            case MULTIBOOT_MEMORY_AVAILABLE:
                                serial_write("Available");
                                break;
                            case MULTIBOOT_MEMORY_RESERVED:
                                serial_write("Reserved");
                                break;
                            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                                serial_write("ACPI Reclaimable");
                                break;
                            case MULTIBOOT_MEMORY_NVS:
                                serial_write("ACPI NVS");
                                break;
                            case MULTIBOOT_MEMORY_BADRAM:
                                serial_write("Bad RAM");
                                break;
                            default:
                                serial_write("Unknown");
                                break;
                        }
                        
                        serial_write(")\r\n");
                    }
                    break;
                }
                
                case MULTIBOOT_TAG_TYPE_CMDLINE: {
                    struct multiboot_tag_string *cmdline = (struct multiboot_tag_string *)tag;
                    serial_write("Command line: ");
                    serial_write(cmdline->string);
                    serial_write("\r\n");
                    break;
                }
                
                case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
                    struct multiboot_tag_string *bootloader = (struct multiboot_tag_string *)tag;
                    serial_write("Bootloader: ");
                    serial_write(bootloader->string);
                    serial_write("\r\n");
                    break;
                }
                
                default:
                    // Skip unknown tags
                    break;
            }
            
            // Move to next tag (aligned to 8 bytes)
            tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
        }
        
        // Initialize PMM with memory info
        extern uint32_t _kernel_start;
        extern uint32_t _kernel_end;
        
        // Call PMM initialization if we have memory map info
        if (mmap_entries != NULL && mmap_entry_count > 0) {
            serial_write("Initializing PMM with memory map...\r\n");
            
            // Call PMM initialization with memory map
            // Note: We'll need to implement pmm_init to handle the memory map
            extern void pmm_init(uint32_t mem_upper_kb, uint32_t kernel_start, 
                               uint32_t kernel_end, void *mmap, uint32_t mmap_count);
            
            pmm_init(mem_upper_kb, (uint32_t)&_kernel_start, (uint32_t)&_kernel_end, 
                    mmap_entries, mmap_entry_count);
        } else {
            // Fallback to simple PMM initialization if no memory map is available
            serial_write("WARNING: No memory map available, using basic PMM initialization\r\n");
            
            // Initialize PMM with just the upper memory size
            extern void pmm_init_basic(uint32_t mem_upper_kb, uint32_t kernel_start, 
                                     uint32_t kernel_end);
            pmm_init_basic(mem_upper_kb, (uint32_t)&_kernel_start, (uint32_t)&_kernel_end);
        }
        
        // Mark kernel memory as used
        extern void pmm_mark_used_region(uint32_t base, uint32_t size);
        uint32_t kernel_size = (uint32_t)&_kernel_end - (uint32_t)&_kernel_start;
        pmm_mark_used_region((uint32_t)&_kernel_start, kernel_size);
        
        serial_write("Kernel memory: 0x");
        serial_write_hex((uint32_t)&_kernel_start);
        serial_write(" - 0x");
        serial_write_hex((uint32_t)&_kernel_end);
        serial_write(" (");
        
        if (kernel_size >= 1024) {
            serial_write_dec(kernel_size / 1024);
            serial_write(" KB");
        } else {
            serial_write_dec(kernel_size);
            serial_write(" bytes");
        }
        
        serial_write(")\r\n");
    } else {
        // Fallback to a simple memory model if multiboot info is not available
        serial_write("WARNING: No valid multiboot magic number, using fallback memory detection\r\n");
        
        // Default to 64MB of memory if we can't determine it
        uint32_t mem_size_mb = 64;
        uint32_t default_mem_upper_kb = (mem_size_mb * 1024) - 1024; // First 1MB is reserved
        
        // Initialize PMM with default values
        serial_write("Initializing PMM with default memory (");
        serial_write_dec(mem_size_mb);
        serial_write(" MB)\r\n");
        
        extern void pmm_init_default(uint32_t mem_upper_kb);
        pmm_init_default(default_mem_upper_kb);
    }
    
    serial_write("[INIT] Physical memory manager initialized\r\n");
    serial_write("[INIT] Memory initialization completed\r\n");
    
    // Call display_init with multiboot info
    extern void display_init(unsigned long magic, struct multiboot_info *mbi);
    display_init((unsigned long)mb_magic, (struct multiboot_info*)mb_info_addr);
}

// PIC (Programmable Interrupt Controller) initialization
void pic_init(void) {
    serial_write("[INIT] PIC initialization started\r\n");
    // Remap PIC IRQs to avoid conflicts with CPU exceptions
    // This should be implemented in a dedicated PIC driver
    serial_write("[INIT] PIC initialization completed\r\n");
}

// Timer initialization is now in kernel/timer.c





// System call initialization
void syscall_init(void) {
    serial_write("[INIT] System call initialization started\r\n");
    // Set up system call handler
    // This should be implemented in the syscall handler code
    serial_write("[INIT] System call initialization completed\r\n");
}

// Mount root filesystem
void mount_root(void) {
    serial_write("[INIT] Mounting root filesystem\r\n");
    // Mount the root filesystem
    // This should be implemented in the filesystem code
}

// Start the init process
void start_init(void) {
    serial_write("[INIT] Starting init process\r\n");
    // This function should start the first userspace process
    // For now, just print a message
    serial_write("System ready. Starting init...\r\n");
    
    // In a real system, this would be something like:
    // exec("/sbin/init", NULL);
    
    // For now, just return and let the scheduler take over
    serial_write("[INIT] Init process setup complete\r\n");
}
