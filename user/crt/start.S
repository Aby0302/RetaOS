.globl _start
.type _start, @function
.extern main
.extern _exit

_start:
    # simple process entry: call main(0, NULL, NULL)
    xor %ebp, %ebp
    push $0      # envp
    push $0      # argv
    push $0      # argc
    call main
    # return value in eax -> exit(status)
    push %eax
    call _exit
    hlt
