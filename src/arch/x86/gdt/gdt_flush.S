.global gdt_flush
gdt_flush:
    lgdt (%eax)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $0x08, $.flush2
.flush2:
    ret

.section .note.GNU-stack,"",@progbits
