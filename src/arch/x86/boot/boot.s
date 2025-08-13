; RetaOS - bootstrap (NASM, Multiboot v1)
%define ALIGN    1<<0
%define MEMINFO  1<<1
%define FLAGS    ALIGN|MEMINFO
%define MAGIC    0x1BADB002
%define CHECKSUM 0xE4524FFB

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384  ; 16 KB
stack_top:

section .text
global _start
extern kernel_main

_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Pass Multiboot registers as cdecl args: (magic, mbi_addr)
    push ebx
    push eax
    
    ; Call kernel main
    call kernel_main
    
    ; Halt the CPU
    cli
.hang:
    hlt
    jmp .hang
