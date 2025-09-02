/* Multiboot info variables */
.global mb_magic
.global mb_info_addr

/* Multiboot header constants */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

/* Multiboot2 header - removed for now, using multiboot1 */

/* 
This part must be 4-byte aligned, hence the .p2align.
*/
.section .multiboot
.p2align 2  /* 2^2 = 4 byte alignment */
.long MAGIC
.long FLAGS
.long CHECKSUM

/*
Allocate the initial stack.
*/
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 /* 16 KiB */
stack_top:

/*
The linker script specifies _start as the entry point to the kernel and the
bootloader will jump to this position once the kernel has been loaded. It
should never return.
*/
.section .text
.global _start
.type _start, @function
_start:
    /* Save multiboot info first thing */
    movl %eax, mb_magic
    movl %ebx, mb_info_addr
    
    /* Early serial port initialization for debugging */
    movw $0x3F8 + 1, %dx
    movb $0x00, %al
    outb %al, %dx
    
    /* Initialize multiboot info structure with framebuffer data */
    movl mb_info_addr, %ebx
    movl $0x1, (%ebx), %eax     /* Set flags to indicate multiboot info is valid */
    movl $0x100000, 4(%ebx)     /* Set mem_lower to 1MB for example */
    movl $0x1000000, 8(%ebx)    /* Set mem_upper to 10MB for example */
    movl $0x1234, 12(%ebx)      /* Set boot_device to dummy value */
    movl $0x1000, 16(%ebx)       /* Set cmdline to dummy address */
    movl $0x0, 20(%ebx)          /* Set mods_count to zero */
    movl $0x0, 24(%ebx)          /* Set mods_addr to zero */
    
    /* Set framebuffer info */
    movl $0xA0000, 28(%ebx)       /* Set framebuffer address */
    movl $0x1000, 32(%ebx)        /* Set framebuffer pitch */
    movl $0x800, 36(%ebx)         /* Set framebuffer width */
    movl $0x600, 40(%ebx)         /* Set framebuffer height */
    movl $0x32, 44(%ebx)          /* Set framebuffer BPP to 32-bit */
    
    movw $0x3F8 + 3, %dx
    movb $0x80, %al
    outb %al, %dx
    
    movw $0x3F8, %dx
    movb $0x01, %al    /* 115200 baud divisor low */
    outb %al, %dx
    
    movw $0x3F8 + 1, %dx
    movb $0x00, %al    /* 115200 baud divisor high */
    outb %al, %dx
    
    movw $0x3F8 + 3, %dx
    movb $0x03, %al    /* 8 bits, no parity, one stop bit */
    outb %al, %dx
    
    movw $0x3F8 + 2, %dx
    movb $0xC7, %al    /* Enable FIFO, clear them, 14-byte threshold */
    outb %al, %dx
    
    movw $0x3F8 + 4, %dx
    movb $0x0B, %al    /* IRQs enabled, RTS/DSR set */
    outb %al, %dx
    
    /* Send debug marker */
    movw $0x3F8, %dx
    movb $'[', %al
    outb %al, %dx
    movb $'B', %al
    outb %al, %dx
    movb $'O', %al
    outb %al, %dx
    movb $'O', %al
    outb %al, %dx
    movb $'T', %al
    outb %al, %dx
    movb $']', %al
    outb %al, %dx
    
    movw $0x3F8 + 1, %dx
    movb $0x00, %al
    outb %al, %dx
    
    movw $0x3F8 + 3, %dx
    movb $0x03, %al    /* 8 bits, no parity, one stop bit */
    outb %al, %dx
    
    movw $0x3F8 + 2, %dx
    movb $0xC7, %al    /* Enable FIFO, clear them, 14-byte threshold */
    outb %al, %dx
    
    movw $0x3F8 + 4, %dx
    movb $0x0B, %al    /* IRQs enabled, RTS/DSR set */
    outb %al, %dx
    
    /* Set up the stack pointer */
    movl $stack_top, %esp
    
    /* Reset EFLAGS */
    pushl $0
    popf
    
    /* Debug: Print stack setup info */
    movw $0x3F8, %dx
    movb $'[', %al
    outb %al, %dx
    movb $'S', %al      /* 'S' for Stack */
    outb %al, %dx
    movb $'P', %al      /* 'P' for Pointer */
    outb %al, %dx
    movb $'=', %al      /* '=' for separator */
    outb %al, %dx
    
    /* Print stack pointer value */
    movl %esp, %eax
    call print_hex32
    
    movb $']', %al      /* Close bracket */
    outb %al, %dx
    movb $'\r', %al    /* New line */
    outb %al, %dx
    movb $'\n', %al
    outb %al, %dx
    
    /* Debug: Print multiboot magic */
    movw $0x3F8, %dx
    movb $'M', %al      /* 'M' for Magic */
    outb %al, %dx
    movb $'B', %al      /* 'B' for Boot */
    outb %al, %dx
    movb $'=', %al      /* '=' for separator */
    outb %al, %dx
    
    /* Print multiboot magic value */
    movl %eax, %ebx     /* Save magic in EBX */
    call print_hex32
    
    movb $'\r', %al    /* New line */
    outb %al, %dx
    movb $'\n', %al
    outb %al, %dx
    
    /* Debug: Print multiboot info address */
    movw $0x3F8, %dx
    movb $'I', %al      /* 'I' for Info */
    outb %al, %dx
    movb $'n', %al      /* 'n' for info */
    outb %al, %dx
    movb $'f', %al      /* 'f' for info */
    outb %al, %dx
    movb $'o', %al      /* 'o' for info */
    outb %al, %dx
    movb $'=', %al      /* '=' for separator */
    outb %al, %dx
    
    /* Print multiboot info address */
    movl %ebx, %eax     /* Restore info pointer from EBX */
    call print_hex32
    
    movb $'\r', %al    /* New line */
    outb %al, %dx
    movb $'\n', %al
    outb %al, %dx
    
    /* Multiboot info is already saved at the beginning */
    
    /* Clear the BSS section */
    /* Debug: Print 'B' before BSS clear */
    movw $0x3F8, %dx
    movb $'B', %al
    outb %al, %dx
    
    movl $__bss_start, %edi
    movl $__bss_end, %ecx
    subl %edi, %ecx
    xorl %eax, %eax
    cld
    rep stosb
    
    /* Debug: Print 'C' after BSS clear */
    movw $0x3F8, %dx
    movb $'C', %al
    outb %al, %dx
    
    /* Debug: Print '3' to show BSS is cleared */
    movw $0x3F8, %dx
    movb $'3', %al
    outb %al, %dx
    
    /* Push parameters for kernel_main (reverse order) */
    pushl mb_info_addr    /* multiboot info pointer (second parameter) */
    pushl mb_magic        /* multiboot magic number (first parameter) */
    
    /* Debug: Print '4' to show we're about to call _init */
    movw $0x3F8, %dx
    movb $'4', %al
    outb %al, %dx
    
    /* Call the global constructors */
    movw $0x3F8, %dx
    movb $'I', %al
    outb %al, %dx
    call _init
    
    /* Initialize display and input subsystems */
    call kernel_init
    
    /* Debug: Print 'M' to show we're about to call _kernel_main */
    movw $0x3F8, %dx
    movb $'M', %al
    outb %al, %dx
    
    /* Call the kernel main function */
    call _kernel_main
    
    /* Debug: Print 'X' if kernel_main returns (shouldn't happen) */
    movw $0x3F8, %dx
    movb $'X', %al
    outb %al, %dx
    
    /* If kernel_main returns, hang */
    cli
1:  hlt
    jmp 1b

/*
 * Print a 32-bit hexadecimal number to the serial port
 * Input: EAX = number to print
 * Clobbers: EAX, EBX, ECX, EDX
 */
print_hex32:
    push %ecx
    push %edx
    
    movl $8, %ecx          /* 8 hex digits to print */
    movw $0x3F8, %dx       /* Serial port base */
    
1:  rol $4, %eax           /* Rotate high nibble to low */
    movb %al, %bl          /* Save rotated value */
    andb $0x0F, %bl        /* Keep only low nibble */
    cmpb $10, %bl          /* Compare with 10 */
    jb 2f                  /* If below 10, it's a digit */
    addb $('A' - 10), %bl  /* Otherwise, it's a letter A-F */
    jmp 3f
2:  addb $'0', %bl         /* Convert to ASCII digit */
3:  xchg %al, %bl          /* Swap AL and BL */
    outb %al, %dx          /* Print character */
    xchg %al, %bl          /* Swap back */
    loop 1b                /* Next digit */
    
    pop %edx
    pop %ecx
    ret

/*
The size of the _start symbol is the current location minus its start.
This is used in the linker script to ensure that _start is at the beginning
of the .text section.
*/
.size _start, . - _start
