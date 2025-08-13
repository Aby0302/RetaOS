.set  MB2_MAGIC, 0xE85250D6
.set  ARCH_I386, 0
.section .multiboot2
.align 8
mb2_start:
  .long MB2_MAGIC
  .long ARCH_I386
  .long mb2_end - mb2_start
  .long -(MB2_MAGIC + ARCH_I386 + (mb2_end - mb2_start))
  .word 0; .word 0; .long 8   # end tag
mb2_end:
.section .bss
.align 16
stack_bottom2: .skip 16384
stack_top2:
.section .text
.global _start_mb2
_start_mb2:
  mov $stack_top2, %esp
  call kernel_main
1: cli; hlt; jmp 1b
