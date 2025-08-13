.global isr14_stub
.extern isr14_handler

.global irq0_stub, irq1_stub, irq2_stub, irq3_stub, irq4_stub, irq5_stub, irq6_stub, irq7_stub
.global irq8_stub, irq9_stub, irq10_stub, irq11_stub, irq12_stub, irq13_stub, irq14_stub, irq15_stub
.extern irq_handler

isr14_stub:
    pushal
    pushl $14
    call isr14_handler
    add $8, %esp
    popal
    iret

irq0_stub:
    pushal
    pushl $0
    call irq_handler
    add $4, %esp
    popal
    iret
irq1_stub:
    pushal
    pushl $1
    call irq_handler
    add $4, %esp
    popal
    iret
irq2_stub:
    pushal
    pushl $2
    call irq_handler
    add $4, %esp
    popal
    iret
irq3_stub:
    pushal
    pushl $3
    call irq_handler
    add $4, %esp
    popal
    iret
irq4_stub:
    pushal
    pushl $4
    call irq_handler
    add $4, %esp
    popal
    iret
irq5_stub:
    pushal
    pushl $5
    call irq_handler
    add $4, %esp
    popal
    iret
irq6_stub:
    pushal
    pushl $6
    call irq_handler
    add $4, %esp
    popal
    iret
irq7_stub:
    pushal
    pushl $7
    call irq_handler
    add $4, %esp
    popal
    iret
irq8_stub:
    pushal
    pushl $8
    call irq_handler
    add $4, %esp
    popal
    iret
irq9_stub:
    pushal
    pushl $9
    call irq_handler
    add $4, %esp
    popal
    iret
irq10_stub:
    pushal
    pushl $10
    call irq_handler
    add $4, %esp
    popal
    iret
irq11_stub:
    pushal
    pushl $11
    call irq_handler
    add $4, %esp
    popal
    iret
irq12_stub:
    pushal
    pushl $12
    call irq_handler
    add $4, %esp
    popal
    iret
irq13_stub:
    pushal
    pushl $13
    call irq_handler
    add $4, %esp
    popal
    iret
irq14_stub:
    pushal
    pushl $14
    call irq_handler
    add $4, %esp
    popal
    iret
irq15_stub:
    pushal
    pushl $15
    call irq_handler
    add $4, %esp
    popal
    iret

.section .note.GNU-stack,"",@progbits
