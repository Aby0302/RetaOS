; IDT loading routine
global idt_load
idt_load:
    lidt [eax]
    ret

; section .note.GNU-stack,"",@progbits
; Commented out for macOS compatibility
