; Multiboot2 and Multiboot1 headers with proper alignment

; --- Multiboot2 header (GRUB2) ---
section .multiboot2
align 8
global _multiboot2_header

_multiboot2_header:
    dd 0xE85250D6      ; magic
    dd 0               ; architecture (i386)
    dd _multiboot2_header_end - _multiboot2_header ; header length
    dd 0x100000000 - (0xE85250D6 + 0 + (_multiboot2_header_end - _multiboot2_header)) ; checksum

    ; Framebuffer request tag (type=5)
    dw 5                ; type
    dw 0                ; flags
    dd 20               ; size
    dd 1024             ; width
    dd 768              ; height
    dd 32               ; depth

    align 8 ; Align the next tag to a 8-byte boundary

    ; End tag
    dw 0                ; type
    dw 0                ; flags
    dd 8                ; size
_multiboot2_header_end:

; --- Multiboot1 header (GRUB legacy) ---
section .multiboot
align 4
global _multiboot_header
_multiboot_header:
    MB1_ALIGN equ 1<<0    ; align loaded modules on page boundaries
    MB1_MEMINFO equ 1<<1    ; provide memory map
    MB1_FLAGS equ MB1_ALIGN | MB1_MEMINFO    ; this is the Multiboot 'flag' field
    MB1_MAGIC equ 0x1BADB002    ; 'magic number' lets bootloader find the header
    MB1_CHECKSUM equ -(MB1_MAGIC + MB1_FLAGS)
    dd MB1_MAGIC
    dd MB1_FLAGS
    dd MB1_CHECKSUM

; --- Stack ---
section .bss
align 16
global _stack_bottom
_stack_bottom:
    resb 16384
global _stack_top
_stack_top:

; --- Code ---
section .text
global _start
extern _kernel_main

_start:
    ; Set up stack pointer
    mov esp, _stack_top
    ; Clear direction flag
    cld
    ; Multiboot passes magic in EAX and info addr in EBX
    push ebx            ; 2nd arg: info address
    push eax            ; 1st arg: magic
    ; Call kernel main
    call _kernel_main
    add esp, 8         ; Clean up arguments
    ; Halt if kernel_main returns
    cli
.halt:
    hlt
    jmp .halt

; Make sure the entry point is at a known location
global _start_boot
_start_boot:
    nop
    jmp _start