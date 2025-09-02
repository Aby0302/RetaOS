; C runtime startup code for RetaOS
; This sets up the environment for main() and calls exit(main(argc, argv))

section .text
    global _start
    extern main
    extern exit
    
_start:
    ; Clear BSS section
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb
    
    ; Get command line arguments from the stack
    pop eax        ; argc
    mov ebx, esp   ; argv
    
    ; Align stack to 16 bytes for SSE
    and esp, 0xFFFFFFF0
    
    ; Push arguments for main
    push ebx       ; argv
    push eax       ; argc
    
    ; Call main
    call main
    
    ; Call exit with main's return value
    push eax
    call exit
    
    ; Should never reach here
    jmp $

section .bss
    ; Mark BSS section boundaries
    global __bss_start
    global __bss_end
    __bss_start:
    resb 0
    __bss_end:
    resb 0
