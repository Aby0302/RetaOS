.global ex0_stub, ex1_stub, ex2_stub, ex3_stub, ex4_stub, ex5_stub, ex6_stub, ex7_stub
.global ex8_stub, ex9_stub, ex10_stub, ex11_stub, ex12_stub, ex13_stub, ex14_stub_pf, ex15_stub
.global ex16_stub, ex17_stub, ex18_stub, ex19_stub
.global syscall80_stub
.extern exception_handler

.global irq0_stub, irq1_stub, irq2_stub, irq3_stub, irq4_stub, irq5_stub, irq6_stub, irq7_stub
.global irq8_stub, irq9_stub, irq10_stub, irq11_stub, irq12_stub, irq13_stub, irq14_stub, irq15_stub
.extern irq_handler
.extern syscall_handler
.extern irq0_handler

# Non-error-code exception stub macro
# arg0: label base (e.g., ex0), arg1: numeric vector (e.g., 0)
.macro EX_NOERR veclabel, vecnum
\veclabel\()_stub:
    pushal
    pushl $0                 # fake error code
    pushl $\vecnum           # vector number
    lea 8(%esp), %eax        # ctx pointer to saved regs
    pushl %eax               # ctx pointer
    call exception_handler
    addl $12, %esp           # discard args (ctx, vec, err)
    popal
    iret
.endm

# Error-code exception stub macro
# arg0: label base (e.g., ex8), arg1: numeric vector (e.g., 8)
.macro EX_ERR veclabel, vecnum
\veclabel\()_stub:
    pushal
    movl 32(%esp), %eax      # CPU error code after pushal
    pushl %eax               # error code
    pushl $\vecnum           # vector number
    lea 8(%esp), %eax        # ctx pointer to saved regs
    pushl %eax               # ctx pointer
    call exception_handler
    addl $12, %esp           # discard args (ctx, vec, err)
    popal
    addl $4, %esp            # discard CPU error code
    iret
.endm

# Define exception stubs 0..19
EX_NOERR ex0, 0
EX_NOERR ex1, 1
EX_NOERR ex2, 2
EX_NOERR ex3, 3
EX_NOERR ex4, 4
EX_NOERR ex5, 5
EX_NOERR ex6, 6
EX_NOERR ex7, 7
EX_ERR   ex8, 8
EX_NOERR ex9, 9
EX_ERR   ex10, 10
EX_ERR   ex11, 11
EX_ERR   ex12, 12
EX_ERR   ex13, 13
# Page Fault uses dedicated name for clarity
ex14_stub_pf:
    pushal
    movl 32(%esp), %eax
    pushl %eax               # error code
    pushl $14                # vector
    lea 8(%esp), %eax        # ctx pointer to saved regs
    pushl %eax
    call exception_handler
    addl $12, %esp
    popal
    addl $4, %esp
    iret
EX_NOERR ex15, 15
EX_NOERR ex16, 16
EX_ERR   ex17, 17
EX_NOERR ex18, 18
EX_NOERR ex19, 19

# Syscall and IRQ stubs
syscall80_stub:
    pushal
    # Pass pointer to saved registers as ctx
    lea 0(%esp), %eax
    pushl %eax
    call syscall_handler
    addl $4, %esp
    # Place return value into saved EAX slot so popal restores it
    mov %eax, 0(%esp)
    popal
    iret
irq0_stub:
    pushal
    # Pass current ESP (pointer to saved regs) to C handler for preemptive switch
    pushl %esp
    call irq0_handler
    addl $4, %esp
    # If C handler returns non-zero in EAX, switch stack to that value
    test %eax, %eax
    jz 1f
    mov %eax, %esp
1:
    popal
    iret
irq1_stub:
    pushal
    pushl $1
    call irq_handler
    addl $4, %esp
    popal
    iret
irq2_stub:
    pushal
    pushl $2
    call irq_handler
    addl $4, %esp
    popal
    iret
irq3_stub:
    pushal
    pushl $3
    call irq_handler
    addl $4, %esp
    popal
    iret
irq4_stub:
    pushal
    pushl $4
    call irq_handler
    addl $4, %esp
    popal
    iret
irq5_stub:
    pushal
    pushl $5
    call irq_handler
    addl $4, %esp
    popal
    iret
irq6_stub:
    pushal
    pushl $6
    call irq_handler
    addl $4, %esp
    popal
    iret
irq7_stub:
    pushal
    pushl $7
    call irq_handler
    addl $4, %esp
    popal
    iret
irq8_stub:
    pushal
    pushl $8
    call irq_handler
    addl $4, %esp
    popal
    iret
irq9_stub:
    pushal
    pushl $9
    call irq_handler
    addl $4, %esp
    popal
    iret
irq10_stub:
    pushal
    pushl $10
    call irq_handler
    addl $4, %esp
    popal
    iret
irq11_stub:
    pushal
    pushl $11
    call irq_handler
    addl $4, %esp
    popal
    iret
irq12_stub:
    pushal
    pushl $12
    call irq_handler
    addl $4, %esp
    popal
    iret
irq13_stub:
    pushal
    pushl $13
    call irq_handler
    addl $4, %esp
    popal
    iret
irq14_stub:
    pushal
    pushl $14
    call irq_handler
    addl $4, %esp
    popal
    iret
irq15_stub:
    pushal
    pushl $15
    call irq_handler
    addl $4, %esp
    popal
    iret

# .section .note.GNU-stack,"",@progbits
# Commented out for macOS compatibility
