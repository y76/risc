
.section .text.boot
.globl _start
_start:
    # Set stack pointer
    la sp, _stack_top

    # Zero .bss
    la a0, __bss_start
    la a1, __bss_end
    li a2, 0
1:
    bge a0, a1, 2f
    sb a2, 0(a0)
    addi a0, a0, 1
    j 1b
2:

    # Dump mhartid
    csrr a0, mhartid

    li t1, 0xFFE7014010  # UART log helper: send 'H' with core ID
    li t2, 'H'
    sb t2, 0(t1)
    addi t2, a0, '0'
    sb t2, 0(t1)

    # Read SYSREG_T at 0xFFF01_8000 (adjusted for readability)
    li t0, 0xFFF018000
    lw t2, 0(t0)
    sb t2, 0(t1)

    # Attempt PMP config
    li t0, 0xFFE0000000 >> 2
    csrw pmpaddr0, t0

    li t0, 0xFFE0180000 >> 2
    csrw pmpaddr1, t0

    li t0, 0x8F  # TOR | R | L
    csrw pmpcfg0, t0

    # Continue into main
    call main
    j .
