ENTRY(_start)

/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */

/* Kernel virtual base address */
KERNEL_VIRTUAL_BASE = 0xC0000000;

SECTIONS
{
    /* Place the multiboot headers at the absolute beginning of the file */
    . = 0x100000; /* 1MB load address in memory */
    .multiboot : ALIGN(8)
    {
        KEEP(*(.multiboot))   /* Keep MB2+MB1 headers */
    }
    _kernel_start = .;

    /* Executable code */
    .text ALIGN(4096) :
    {
        *(.text*)
    }

    /* Read-only data */
    .rodata ALIGN(4096) :
    {
        *(.rodata*)
    }

    /* Read-write data (initialized) */
    .data ALIGN(4096) :
    {
        *(.data*)
        . = ALIGN(4);
        mb_magic = .;      /* exported for early debug (optional) */
        . += 4;
        mb_info_addr = .;  /* exported for early debug (optional) */
        . += 4;
    }

    /* Read-write data (uninitialized) and stack */
    .bss ALIGN(4096) :
    {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
    }
    _kernel_end = .;

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}
