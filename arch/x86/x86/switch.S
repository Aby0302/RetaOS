global context_switch
; void context_switch(uint32_t* old_esp, uint32_t new_esp)
; Saves current ESP into *old_esp, loads new ESP, and returns to the caller
; on the new stack. The new stack must have a return address prepared.
context_switch:
    ; Arguments: [esp+4]=old_esp, [esp+8]=new_esp
    mov eax, [esp + 4]      ; eax = old_esp*
    mov edx, esp         ; edx = current esp (return address on top)
    mov [eax], edx       ; *old_esp = current esp
    mov ecx, [esp + 8]      ; ecx = new_esp
    mov esp, ecx         ; switch stacks
    ret                    ; continue execution on new stack

; void switch_threads(thread_t* from, thread_t* to)
; Saves the current thread context and restores the new thread context
global switch_threads
switch_threads:
    ; Save current thread context
    mov eax, [esp + 4]      ; eax = from thread
    mov ecx, [esp + 8]      ; ecx = to thread
    
    ; Save non-volatile registers
    push ebp
    push ebx
    push esi
    push edi
    
    ; Save current stack pointer to from->stack (offset 4 in thread_t)
    mov [eax + 4], esp      ; from->stack = esp
    
    ; Load new stack pointer from to->stack (offset 4 in thread_t)
    mov esp, [ecx + 4]      ; esp = to->stack
    
    ; Restore non-volatile registers
    pop edi
    pop esi
    pop ebx
    pop ebp
    
    ; Return to the new thread's execution
    ret

; section .note.GNU-stack,"",@progbits
; Commented out for macOS compatibility
