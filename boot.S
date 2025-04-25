
    .section .text.boot
    .globl _start
    .globl _boot

_boot:
    la sp, _stack_top

    la a0, __bss_start
    la a1, __bss_end
1:
    bge a0, a1, 2f
    sd zero, 0(a0)
    addi a0, a0, 8
    j 1b
2:
    tail _start

    .section .bss.stack
    .align 16
    .space 0x1000
_stack_top:
