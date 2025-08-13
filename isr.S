.global isr14_stub
.extern isr14_handler
isr14_stub:
    pusha
    pushl $14
    call isr14_handler
    add $4, %esp
    popa
    iretd
